#include <iostream>
#include <queue>
#include <string>
#include <cstdlib>
		 
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/algorithm/string.hpp>

#include "../DnDShared/globals.h"
#include "accounts.h"
#include "map.h"
#include "../DnDShared/gui.h"

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

typedef ClientMessage User;

// GLOBALS
INTERNAL io_service service;
//127.0.0.1
INTERNAL tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 8001);
INTERNAL boost::mutex generalMutex;
INTERNAL boost::mutex userListMutex;
INTERNAL std::vector<User> userList;
INTERNAL bool closeThreads = false;
INTERNAL bool windowOpen = false;
#define INPUT_SIZE 256

INTERNAL Account account;
INTERNAL StringList rollLog;
INTERNAL bool menace = false;
INTERNAL vec2 menacingPos;
INTERNAL Map map;
INTERNAL i32 roundabout = -1;
// END GLOBALS

// Function Prototypes
INTERNAL void main_loop(Socket* socket);
INTERNAL void receive_loop(Socket* socket);
INTERNAL void send_handler(const boost::system::error_code& error, std::size_t bytes_transferred);

INTERNAL void map_input(Map* map);
INTERNAL void draw_log(RenderBatch* batch);
INTERNAL void roll_prompt(RenderBatch* batch, Socket* sock, GameState& state);

// End of Function Prototypes

INTERNAL inline
i32 hashpass(const char* str, u32 len) {
	/* by Peter J. Weinberger */
	const u32 BitsInUnsignedInt = (u32)(sizeof(u32) * 8);
	const u32 ThreeQuarters = (u32)((BitsInUnsignedInt * 3) / 4);
	const u32 OneEighth = (u32)(BitsInUnsignedInt / 8);
	const u32 HighBits =
		(u32)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
	u32 hash = 0;
	u32 test = 0;
	u32 i = 0;

	for (i = 0; i < len; ++str, ++i)
	{
		hash = (hash << OneEighth) + (*str);

		if ((test = hash & HighBits) != 0)
		{
			hash = ((hash ^ (test >> ThreeQuarters)) & (~HighBits));
		}
	}

	return hash;
}

INTERNAL
std::string get_input(std::string message) {
	const int MAX_INPUT_SIZE = 256;
	char buffer[MAX_INPUT_SIZE] = { 0 };
	std::cout << message;
	std::cin.getline(buffer, MAX_INPUT_SIZE);
	std::string str(buffer);
	if (str.find('|') != std::string::npos) {
		return get_input("Input cannot have any '|' characters, try again: ");
	}
	return str;
}

int main() {
	try {
		boost::thread_group threads;
		Socket* sock = new tcp::socket(service);

		std::cout << "Please log in. If an account does not exist using the entered username, it will be created for you." << std::endl;
		std::string namebuffer = get_input("Enter username: ");
		std::string name = "name|";
		name.append(namebuffer);

		std::string passbuffer = get_input("Enter password: ");
		i32 hash = hashpass(passbuffer.c_str(), strlen(passbuffer.c_str()));
		name.append("|pass|");

		account.name = namebuffer;
		account.pass = to_string(hash);
		name.append(to_string(hash));

		User user;
		user.str = namebuffer;
		user.socket = sock;
		userList.push_back(user);

		sock->connect(ep);
		sock->write_some(buffer(name, name.size()));

		std::cout << "Successfully connected to server on port 8001\n" << std::endl;
		threads.create_thread(boost::bind(main_loop, sock));
		threads.create_thread(boost::bind(receive_loop, sock));

		threads.join_all();
		sock->write_some(buffer("exit\n", 4));
		delete sock;
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
#ifdef _WIN32
		std::system("PAUSE");
#endif
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

INTERNAL
void send_handler(const boost::system::error_code& error, std::size_t bytes_transferred) {

}

INTERNAL
void handle_message(Socket* sock, StringList* tokens) {
	if (tokens->at(0) == "name") {
		BMT_LOG(INFO, "User '%s' has connected", tokens->at(1).c_str());
		userListMutex.lock();
		User user;
		user.socket = NULL;
		user.str = tokens->at(1);

		userList.push_back(user);
		userListMutex.unlock();
	}
	else if (tokens->at(0) == "login_failure") {
		BMT_LOG(WARNING, "Failed to login. Account exists but the password is incorrect. Program will close.");
		boost::this_thread::sleep(boost::posix_time::millisec(3000));
		closeThreads = true;
	}
	else if (tokens->at(0) == "login_success") {
		BMT_LOG(INFO, "Successfully logged in!\n");
		load_account(&account, tokens);
		boost::this_thread::sleep(boost::posix_time::millisec(2000));
		windowOpen = true;
	}
	else if (tokens->at(0) == "login_created") {
		BMT_LOG(INFO, "Created a new account! Logging in...\n");
		load_account(&account, tokens);
		boost::this_thread::sleep(boost::posix_time::millisec(3000));
		windowOpen = true;
	}
	if (tokens->at(0) == "roll") {
		if (std::atoi(tokens->at(1).c_str()) == 1) {
			std::string str = tokens->at(2);
			str.append(" rolled a dice(1-6): ");
			str.append(tokens->at(3));
			rollLog.push_back(str);
			if (rollLog.size() > 15) {
				rollLog.erase(rollLog.begin());
			}
		}
	}
	if (tokens->at(0) == "play_music") {
		play_sound(music[ std::atoi( tokens->at(1).c_str() ) ]);
	}
	if (tokens->at(0) == "menacing") {
		menace = true;
		menacingPos = V2(-100, -100);
	}
	if (tokens->at(0) == "move") {
		map.tokens[std::atoi( tokens->at(1).c_str() )].xPos = std::atoi( tokens->at(2).c_str() );
		map.tokens[std::atoi( tokens->at(1).c_str() )].yPos = std::atoi( tokens->at(3).c_str() );
	}
	if (tokens->at(0) == "update_token") {
		i32 ndx = std::stoi(tokens->at(1));
		if (ndx == -1) {
			Token token = { 0 };
			map.tokens.push_back(token);
			ndx = map.tokens.size() - 1;
		}
		map.tokens[ndx].bar1.current = std::stoi(tokens->at(2));
		map.tokens[ndx].bar1.max = std::stoi(tokens->at(3));
		map.tokens[ndx].bar2.current = std::stoi(tokens->at(4));
		map.tokens[ndx].bar2.max = std::stoi(tokens->at(5));
		map.tokens[ndx].bar3.current = std::stoi(tokens->at(6));
		map.tokens[ndx].bar3.max = std::stoi(tokens->at(7));
		map.tokens[ndx].name = tokens->at(8);
		map.tokens[ndx].imgindex = std::stoi(tokens->at(9));
	}
	if (tokens->at(0) == "update_map") {
		map.tokens.clear();
		map.rects.clear();
		map = { 0 };

		map.width = std::stoi(tokens->at(1));
		map.height = std::stoi(tokens->at(2));
		map.xPos = std::stoi(tokens->at(3));
		map.yPos = std::stoi(tokens->at(4));
		map.grid = std::stoi(tokens->at(5));
		map.fow = std::stoi(tokens->at(6));
		map.bgColor = { std::stof(tokens->at(10)), std::stof(tokens->at(9)), std::stof(tokens->at(8)), std::stof(tokens->at(7)) };
		map.gridColor = { std::stof(tokens->at(14)), std::stof(tokens->at(13)), std::stof(tokens->at(12)), std::stof(tokens->at(11)) };
		//todo: fix colors
		map.bgColor = WHITE;
		map.gridColor = GRAY;
		map.selected = -1;
	}
	if (tokens->at(0) == "roundabout") {
		roundabout = 2660;
	}
}

INTERNAL
void receive_loop(Socket* sock) {
	for (;;) {
		if (closeThreads) break;

		if (sock->available()) {
			generalMutex.lock();
			char readBuffer[BUFFER_SIZE] = { 0 };
			int bytesRead = sock->read_some(buffer(readBuffer, BUFFER_SIZE));
			std::string msg = std::string(readBuffer, bytesRead);

			BMT_LOG(DEBUG, "Received response from server: %s", msg.c_str());

			StringList commands = split_string(msg, '\n');
			for (u16 i = 0; i < commands.size(); ++i) {
				StringList tokens = split_string(commands[i], '|');
				if (tokens.size() > 0) {
					handle_message(sock, &tokens);
				}
			}
			generalMutex.unlock();
		}

		boost::this_thread::sleep(boost::posix_time::millisec(1000));
	}
	generalMutex.lock();
	BMT_LOG(INFO, "Closed receive_loop");
	generalMutex.unlock();
}

INTERNAL
void draw_usernames(RenderBatch* batch) {
	generalMutex.lock();
	for (u16 i = 0; i < userList.size(); ++i) {
		i32 x = (i * 110) + 20;
		i32 y = get_window_height() - 40;
		i32 width = get_string_width(BODY_FONT, userList[i].str.c_str()) + 8;
		i32 height = 30;

		draw_rectangle(batch, x, y, width, height, FADED_RED);
		draw_text(batch, &BODY_FONT, userList[i].str, x + 5, y + (height / 2) - (BODY_FONT.characters['t']->texture.height / 2), 255, 255, 255);
	}
	generalMutex.unlock();
}

const u8 NUM_COLORS = 9;

INTERNAL inline
TextField create_textfield(i32 width, i32 height, u16 maxChars, u16 maxLines, TextFieldType type) {
	TextField test;
	test.maxChars = maxChars;
	test.maxLines = maxLines;
	test.text.push_back("");
	test.bgcolor = DARKGRAY;
	test.fgcolor = WHITE;
	test.width = width;
	test.height = height;
	test.textcolor = DARKGRAY.xyz;
	test.type = type;
	return test;
}

INTERNAL inline
void draw_outline(RenderBatch* batch, f32 x, f32 y, f32 width, f32 height) {
	draw_rectangle(batch, x, y, width, height, GRAY);
	draw_rectangle(batch, x + 5, y + 5, width - 10, height - 10, WHITE);
}

INTERNAL inline
void set_bar_value(StatusBar* bar, TextField* field1, TextField* field2) {
	if (field1->text.size() > 0 && field1->text[0] != "")
		bar->current = std::stoi(field1->text[0]);
	if (field2->text.size() > 0 && field2->text[0] != "")
		bar->max = std::stoi(field2->text[0]);
}

INTERNAL
void main_loop(Socket* socket) {
 	init_window(1400, 800, "Jojo Tabletop", false, true, true);
	init_audio();
	set_fps_cap(60);
	set_vsync(false);
	set_master_volume(100);
	set_clear_color(FADED_TEAL);
	set_mouse_state(MOUSE_HIDDEN);

	map = { 0 };
	map.selected = -1;
	map.width = map.height = 20;
	map.bgColor = WHITE;
	map.gridColor = GRAY;
	Token token = { 0 };
	token.xPos = token.yPos = 128;
	map.tokens.push_back(token);

	RenderBatch* batch = &create_batch();
	Shader basic = load_default_shader_2D();
	printf("\n\n");
	GameState state = STATE_IDLE;
	load_all_textures();

	vec4 colors[NUM_COLORS] = { BLACK, WHITE, RED, BLUE, GREEN, FADED_GREEN, FADED_RED, FADED_BLUE, FADED_BLACK };
	Font font = load_font("art/opensans-regular.ttf", 24, GL_LINEAR);

	Panel panel = { 0 };

	//textfields for token
	TextField nameField = create_textfield(350, 30, 22, 1, INPUT_EVERYTHING);
	TextField bar11 = create_textfield(63, 30, 4, 1, INPUT_NUMBERS_ONLY);
	TextField bar12 = create_textfield(63, 30, 4, 1, INPUT_NUMBERS_ONLY);
	TextField bar21 = create_textfield(63, 30, 4, 1, INPUT_NUMBERS_ONLY);
	TextField bar22 = create_textfield(63, 30, 4, 1, INPUT_NUMBERS_ONLY);
	TextField bar31 = create_textfield(63, 30, 4, 1, INPUT_NUMBERS_ONLY);
	TextField bar32 = create_textfield(63, 30, 4, 1, INPUT_NUMBERS_ONLY);
	TextField imageField = create_textfield(35, 30, 2, 1, INPUT_NUMBERS_ONLY);

	//textfields for stand user sheet
	TextField standUserName = create_textfield(350, 30, 22, 1, INPUT_EVERYTHING);
	TextField playerName = create_textfield(350, 30, 22, 1, INPUT_EVERYTHING);
	TextField age = create_textfield(100, 30, 6, 1, INPUT_NUMBERS_ONLY);
	TextField gender = create_textfield(100, 30, 6, 1, INPUT_EVERYTHING);
	TextField weightField = create_textfield(100, 30, 6, 1, INPUT_NUMBERS_ONLY);
	TextField heightField = create_textfield(100, 30, 6, 1, INPUT_NUMBERS_ONLY);
	TextField bloodType = create_textfield(100, 30, 6, 1, INPUT_EVERYTHING);
	TextField occupation = create_textfield(250, 30, 22, 1, INPUT_EVERYTHING);
	TextField nationality = create_textfield(250, 30, 22, 1, INPUT_EVERYTHING);

	TextField brains = create_textfield(100, 30, 6, 1, INPUT_NUMBERS_ONLY);
	TextField brawns = create_textfield(100, 30, 6, 1, INPUT_NUMBERS_ONLY);
	TextField bravery = create_textfield(100, 30, 6, 1, INPUT_NUMBERS_ONLY);
	TextField totalHealth = create_textfield(100, 30, 6, 1, INPUT_NUMBERS_ONLY);
	TextField currentHealth = create_textfield(250, 30, 6, 1, INPUT_NUMBERS_ONLY);
	TextField resolveDamage = create_textfield(250, 30, 6, 1, INPUT_NUMBERS_ONLY);
	TextField bizarrePoints = create_textfield(250, 30, 6, 1, INPUT_NUMBERS_ONLY);
	TextField charTypes = create_textfield(490, 30, 30, 1, INPUT_EVERYTHING);
	TextField charBackstory = create_textfield(400, 200, 300, 20, INPUT_EVERYTHING);
	TextField inventory = create_textfield(400, 200, 22, 1, INPUT_EVERYTHING);

	//textfields for stand sheet
	TextField standName = create_textfield(450, 30, 22, 1, INPUT_EVERYTHING);
	TextField standTypes = create_textfield(450, 30, 22, 1, INPUT_EVERYTHING);
	TextField standDesc = create_textfield(500, 400, 700, 100, INPUT_EVERYTHING);

	f64 zoom = .25;
	while (window_open()) {
		if (closeThreads) break;

		vec2 mousePos = get_mouse_pos();

		f32 scrollY = get_scroll_y();
		zoom += scrollY * 0.015625;

		map_input(&map);

		f32 width = (f32)get_window_width();
		f32 height = (f32)get_window_height();

		mat4 ortho = orthographic_projection(0, 0, width, height, -1, 1);
		begin_drawing();
		begin2D(batch, basic);
			set_viewport(0, 0, width, height);
			upload_mat4(basic, "projection", ortho * scale(zoom, zoom, 1));
			draw_map(batch, &map, zoom);
			if (state == STATE_IDLE)
				update_map(batch, &map, socket, state, zoom);
			draw_tokens(batch, &map, zoom);
		end2D(batch);
		begin2D(batch, basic);
		begin_gui(&panel);

			upload_mat4(basic, "projection", ortho);
			draw_usernames(batch);
			if (state != STATE_CHARSHEET && state != STATE_STANDSHEET) {
				draw_log(batch);
				//draw buttons
				if (draw_text_button(batch, "Roll Dice", width - (250 / 2) - (button_tex_n.width / 1.5), height - 50, FADED_RED, WHITE.xyz)) {
					state = STATE_ROLL_PROMPT;
				}
				i32 yPos = -30;
				if (draw_icon_button(batch, &select_button, 10, yPos += 34, 1)) {
					state = STATE_IDLE;
				}
				if (draw_icon_button(batch, &turn_button, 10, yPos += 34, 1)) {

				}
				if (draw_icon_button(batch, &char_sheet_icon, 10, yPos += 34, 1)) {
					standUserName.text[0] = account.usersheet.name;
					playerName.text[0] = account.usersheet.playername;
					gender.text[0] = account.usersheet.gender;
					weightField.text[0] = to_string(account.usersheet.weight);
					heightField.text[0] = to_string(account.usersheet.height);
					bloodType.text[0] = account.usersheet.bloodType;
					occupation.text[0] = account.usersheet.occupation;
					nationality.text[0] = account.usersheet.nationality;
					//get_text(&charBackstory) = account.usersheet.backstory;
					//get_text(&inventory) = account.usersheet.inventory;

					brains.text[0] = to_string(account.usersheet.brains);
					brawns.text[0] = to_string(account.usersheet.brawns);
					bravery.text[0] = to_string(account.usersheet.bravery);
					age.text[0] = to_string(account.usersheet.age);
					totalHealth.text[0] = to_string(account.usersheet.totalHealth);
					currentHealth.text[0] = to_string(account.usersheet.currentHealth);
					resolveDamage.text[0] = to_string(account.usersheet.resolveDamage);
					bizarrePoints.text[0] = to_string(account.usersheet.bizarrePoints);

					standName.text[0] = account.standsheet.name;
					standTypes.text[0] = account.standsheet.standTypes;
					standDesc.text[0] = account.standsheet.standAbilityDesc;
					state = STATE_CHARSHEET;
				}
			}
			if (state == STATE_TOKEN_TRANSITION) {
				Token* current = &map.tokens[map.selected];
				bar11.text[0] = format_text("%d", current->bar1.current);
				bar12.text[0] = format_text("%d", current->bar1.max);
				bar21.text[0] = format_text("%d", current->bar2.current);
				bar22.text[0] = format_text("%d", current->bar2.max);
				bar31.text[0] = format_text("%d", current->bar3.current);
				bar32.text[0] = format_text("%d", current->bar3.max);
				nameField.text[0] = current->name;
				imageField.text[0] = format_text("%d", current->imgindex);
				state = STATE_TOKEN;
			}
			if (state == STATE_TOKEN) {
				Token* current = &map.tokens[map.selected];
				f32 width = 400;
				f32 height = 450;
				f32 xPos = (f32)(get_window_width() / 2) - (width / 2);
				f32 yPos = (f32)(get_window_height() / 2) - (height / 2);

				draw_outline(batch, xPos, yPos, width, height);

				draw_text(batch, &font, "Edit Token", xPos + 135, yPos + 15, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_rectangle(batch, xPos, yPos + 40, width, 4, GRAY);
				draw_text(batch, &font, "Name", xPos + 15, yPos + 55, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &nameField, xPos + 15, yPos + 80);

				draw_text(batch, &font, "Bar 1", xPos + 15, yPos + 115, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &bar11, xPos + 15, yPos + 140);
				draw_text(batch, &font, "/", xPos + 79, yPos + 145, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &bar12, xPos + 90, yPos + 140);

				draw_text(batch, &font, "Bar 2", xPos + 15, yPos + 180, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &bar21, xPos + 15, yPos + 210);
				draw_text(batch, &font, "/", xPos + 79, yPos + 215, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &bar22, xPos + 90, yPos + 210);

				draw_text(batch, &font, "Bar 3", xPos + 15, yPos + 250, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &bar31, xPos + 15, yPos + 280);
				draw_text(batch, &font, "/", xPos + 79, yPos + 285, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &bar32, xPos + 90, yPos + 280);

				draw_text(batch, &font, format_text("Token Image number (0-%d)", TOKEN_IMAGE_COUNT), xPos + 15, yPos + 320, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &imageField, xPos + 15, yPos + 350);

				if (draw_text_button(batch, "Save Changes", xPos + 15, yPos + 390, FADED_RED, WHITE.xyz)) {
					set_bar_value(&map.tokens[map.selected].bar1, &bar11, &bar12);
					set_bar_value(&map.tokens[map.selected].bar2, &bar21, &bar22);
					set_bar_value(&map.tokens[map.selected].bar3, &bar31, &bar32);

					if (imageField.text.size() >= 0 && imageField.text[0] != "") {
						i32 imageNum = std::stoi(imageField.text[0]);
						if (imageNum > 0 && imageNum < TOKEN_IMAGE_COUNT)
							map.tokens[map.selected].imgindex = imageNum;
					}
					map.tokens[map.selected].name = nameField.text[0];
					std::string command = format_text("update_token|%d|%d|%d|%d|%d|%d|%d|%s|%d\n",
						map.selected, current->bar1.current, current->bar1.max, current->bar2.current, current->bar2.max,
						current->bar3.current, current->bar3.max, current->name.c_str(), current->imgindex
					);
					socket->write_some(boost::asio::buffer(command, command.size()));
					state = STATE_IDLE;
				}
				if (draw_text_button(batch, "Cancel", xPos + 185, yPos + 390, FADED_RED, WHITE.xyz)) {
					state = STATE_IDLE;
				}
			}
			if (state == STATE_STANDSHEET) {
				f32 width = 700;
				f32 height = 750;
				f32 xPos = (f32)(get_window_width() / 2) - (width / 2);
				f32 yPos = (f32)(get_window_height() / 2) - (height / 2);

				draw_outline(batch, xPos, yPos, width, height);

				draw_text(batch, &font, "Edit Stand Character Sheet", xPos + 205, yPos += 25, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_rectangle(batch, xPos, yPos + 40, width, 4, GRAY);
				draw_text(batch, &font, "Stand Name", xPos + 15, yPos += 50, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &standName, xPos + 15, yPos += 30);
				draw_text(batch, &font, "Stand Types", xPos + 15, yPos += 50, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &standTypes, xPos + 15, yPos += 30);
				draw_text(batch, &font, "Stand Ability/Description", xPos + 15, yPos += 50, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &standDesc, xPos + 15, yPos += 30);

				if (draw_text_button(batch, "Change Char Sheets", xPos + 285, yPos += 420, FADED_RED, WHITE.xyz)) {
					state = STATE_CHARSHEET;
				}
				if (draw_text_button(batch, "Save Changes", xPos + 15, yPos, FADED_RED, WHITE.xyz)) {
					account.standsheet.name = standName.text[0];
					account.standsheet.standTypes = get_text(&standTypes);
					account.standsheet.standAbilityDesc = get_text(&standDesc);

					std::string command = format_text("update_account|%s|%s|%s|%s|%s|%d|%d|%d|%d|%d|%d|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%d|%d|%d|%d|%d|%d|%d|%d\n",
						account.name.c_str(), account.pass.c_str(), account.standsheet.name.c_str(), account.standsheet.standTypes.c_str(), 
						account.standsheet.standAbilityDesc.c_str(), account.standsheet.speed, account.standsheet.power, account.standsheet.range, 
						account.standsheet.precision, account.standsheet.durability, account.standsheet.learning, account.usersheet.name.c_str(), 
						account.usersheet.playername.c_str(), account.usersheet.gender.c_str(), account.usersheet.weight.c_str(), account.usersheet.height.c_str(),
						account.usersheet.bloodType.c_str(), account.usersheet.occupation.c_str(), account.usersheet.nationality.c_str(), account.usersheet.backstory.c_str(),
						account.usersheet.inventory.c_str(), account.usersheet.brains, account.usersheet.brawns, account.usersheet.bravery, account.usersheet.age,
						account.usersheet.totalHealth, account.usersheet.currentHealth, account.usersheet.resolveDamage, account.usersheet.bizarrePoints
					);
					socket->write_some(boost::asio::buffer(command, command.size()));
					state = STATE_IDLE;
				}
				if (draw_text_button(batch, "Cancel", xPos + 185, yPos, FADED_RED, WHITE.xyz)) {
					state = STATE_IDLE;
				}
			}
			if (state == STATE_CHARSHEET) {
				f32 width = 700;
				f32 height = 850;
				f32 xPos = (f32)(get_window_width() / 2) - (width / 2);
				f32 yPos = (f32)(get_window_height() / 2) - (height / 2);

				draw_outline(batch, xPos, yPos, width, height);

				draw_text(batch, &font, "Edit Stand User Character Sheet", xPos + 185, yPos += 25, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_rectangle(batch, xPos, yPos + 40, width, 4, GRAY);
				draw_text(batch, &font, "Stand User Name", xPos + 15, yPos += 50, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text(batch, &font, "Age", xPos + 400, yPos, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text(batch, &font, "Gender", xPos + 550, yPos, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &standUserName, xPos + 15, yPos += 30);
				draw_text_field(batch, &panel, font, &age, xPos + 400, yPos);
				draw_text_field(batch, &panel, font, &gender, xPos + 550, yPos);

				draw_text(batch, &font, "Player Name", xPos + 15, yPos += 35, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text(batch, &font, "Weight", xPos + 400, yPos, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text(batch, &font, "Height", xPos + 550, yPos, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &playerName, xPos + 15, yPos += 30);
				draw_text_field(batch, &panel, font, &weightField, xPos + 400, yPos);
				draw_text_field(batch, &panel, font, &heightField, xPos + 550, yPos);

				draw_text(batch, &font, "Character Types", xPos + 15, yPos += 35, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text(batch, &font, "Blood Type", xPos + 550, yPos, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &charTypes, xPos + 15, yPos += 30);
				draw_text_field(batch, &panel, font, &bloodType, xPos + 550, yPos);

				draw_text(batch, &font, "Character Backstory", xPos + 15, yPos += 35, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text(batch, &font, "Occupation", xPos + 430, yPos, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text(batch, &font, "Nationality", xPos + 430, yPos + 70, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text(batch, &font, "Brains", xPos + 430, yPos + 140, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text(batch, &font, "Brawns", xPos + 550, yPos + 140, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text(batch, &font, "Bravery", xPos + 430, yPos + 210, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text(batch, &font, "Total Health", xPos + 550, yPos + 210, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text(batch, &font, "Current Health", xPos + 430, yPos + 280, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text(batch, &font, "Resolve Damage", xPos + 430, yPos + 345, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text(batch, &font, "Bizarre Points", xPos + 430, yPos + 415, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &charBackstory, xPos + 15, yPos += 30);
				draw_text_field(batch, &panel, font, &occupation, xPos + 430, yPos);
				draw_text_field(batch, &panel, font, &nationality, xPos + 430, yPos + 70);
				draw_text_field(batch, &panel, font, &brains, xPos + 430, yPos + 140);
				draw_text_field(batch, &panel, font, &brawns, xPos + 550, yPos + 140);
				draw_text_field(batch, &panel, font, &bravery, xPos + 550, yPos + 210);
				draw_text_field(batch, &panel, font, &totalHealth, xPos + 430, yPos + 210);
				draw_text_field(batch, &panel, font, &currentHealth, xPos + 430, yPos + 280);
				draw_text_field(batch, &panel, font, &resolveDamage, xPos + 430, yPos + 345);
				draw_text_field(batch, &panel, font, &bizarrePoints, xPos + 430, yPos + 415);

				draw_text(batch, &font, "Inventory", xPos + 15, yPos += 205, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &inventory, xPos + 15, yPos += 30);

				if (draw_text_button(batch, "Change Char Sheets", xPos + 285, yPos += 230, FADED_RED, WHITE.xyz)) {
					state = STATE_STANDSHEET;
				}
				if (draw_text_button(batch, "Save Changes", xPos + 15, yPos, FADED_RED, WHITE.xyz)) {
					account.usersheet.name = standUserName.text[0];
					account.usersheet.playername = playerName.text[0];
					account.usersheet.gender = gender.text[0];
					account.usersheet.weight = weightField.text[0];
					account.usersheet.height = heightField.text[0];
					account.usersheet.bloodType = bloodType.text[0];
					account.usersheet.occupation = occupation.text[0];
					account.usersheet.nationality = nationality.text[0];
					account.usersheet.backstory = get_text(&charBackstory);
					account.usersheet.inventory = get_text(&inventory);

					account.usersheet.brains = brains.text[0].size() > 0 ? std::stoi(brains.text[0]) : 0;
					account.usersheet.brawns = brawns.text[0].size() > 0 ? std::stoi(brawns.text[0]) : 0;
					account.usersheet.bravery = bravery.text[0].size() > 0 ? std::stoi(bravery.text[0]) : 0;
					account.usersheet.age = age.text[0].size() > 0 ? std::stoi(age.text[0]) : 0;
					account.usersheet.totalHealth = totalHealth.text[0].size() > 0 ? std::stoi(totalHealth.text[0]) : 0;
					account.usersheet.currentHealth = currentHealth.text[0].size() > 0 ? std::stoi(currentHealth.text[0]) : 0;
					account.usersheet.resolveDamage = resolveDamage.text[0].size() > 0 ? std::stoi(resolveDamage.text[0]) : 0;
					account.usersheet.bizarrePoints = bizarrePoints.text[0].size() > 0 ? std::stoi(bizarrePoints.text[0]) : 0;

					std::string command = format_text("update_account|%s|%s|%s|%s|%s|%d|%d|%d|%d|%d|%d|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%d|%d|%d|%d|%d|%d|%d|%d\n",
						account.name.c_str(), account.pass.c_str(), account.standsheet.name.c_str(), account.standsheet.standTypes.c_str(),
						account.standsheet.standAbilityDesc.c_str(), account.standsheet.speed, account.standsheet.power, account.standsheet.range,
						account.standsheet.precision, account.standsheet.durability, account.standsheet.learning, account.usersheet.name.c_str(),
						account.usersheet.playername.c_str(), account.usersheet.gender.c_str(), account.usersheet.weight.c_str(), account.usersheet.height.c_str(),
						account.usersheet.bloodType.c_str(), account.usersheet.occupation.c_str(), account.usersheet.nationality.c_str(), account.usersheet.backstory.c_str(),
						account.usersheet.inventory.c_str(), account.usersheet.brains, account.usersheet.brawns, account.usersheet.bravery, account.usersheet.age,
						account.usersheet.totalHealth, account.usersheet.currentHealth, account.usersheet.resolveDamage, account.usersheet.bizarrePoints
					);
					socket->write_some(boost::asio::buffer(command, command.size()));
					state = STATE_IDLE;
				}
				if (draw_text_button(batch, "Cancel", xPos + 185, yPos, FADED_RED, WHITE.xyz)) {
					state = STATE_IDLE;
				}
			}

			if (menace) {
				draw_texture(batch, menacing, menacingPos.x, menacingPos.y);
				menacingPos.x += random_int(0, 5);
				menacingPos.y += random_int(0, 5);
				if (menacingPos == V2(get_window_width(), get_window_height())) {
					menace = false;
				}
			}
			if (roundabout > 0) {
				roundabout--;
			}
			if (state == STATE_ROLL_PROMPT) {
				roll_prompt(batch, socket, state);
			}
			if (roundabout == 0) {
				draw_rectangle(batch, 0, 0, get_window_width(), get_window_height(), V4(130, 94, 3, 128));
				draw_texture(batch, to_be_continued, 0, get_window_height() - to_be_continued.height);
				if (is_sound_stopped(music[1])) {
					break;
				}
			}
			draw_texture(batch, cursor, (i32)mousePos.x, (i32)mousePos.y);
		
		end_gui(&panel);
		end2D(batch);
		end_drawing();
	}
	closeThreads = true;
	BMT_LOG(INFO, "Closed main_loop");
	dispose_window();
}

INTERNAL
void map_input(Map* map) {
	if (is_key_down(KEY_RIGHT)) {
		map->xPos -= SCROLL_SPEED;
	}
	if (is_key_down(KEY_LEFT)) {
		map->xPos += SCROLL_SPEED;
	}
	if (is_key_down(KEY_DOWN)) {
		map->yPos -= SCROLL_SPEED;
	}
	if (is_key_down(KEY_UP)) {
		map->yPos += SCROLL_SPEED;
	}
}

INTERNAL 
void draw_log(RenderBatch* batch) {
	f32 width = (f32)get_window_width();
	f32 height = (f32)get_window_height();

	draw_rectangle(batch, width - 250, 0, 250, height, WHITE);
	draw_rectangle(batch, width - 254, 0, 4, height, GRAY);
	for (u16 i = 0; i < rollLog.size(); ++i) {
		draw_text(batch, &BODY_FONT, rollLog[i], width - 240, 10 + (i * 25), DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
	}
}

//decide whether to roll privately or publically
INTERNAL
void roll_prompt(RenderBatch* batch, Socket* socket, GameState& state) {
	f32 width = (f32)get_window_width();
	f32 height = (f32)get_window_height();

	draw_outline(batch, (width / 2) - (button_tex_n.width / 2) - 190 - 30, (height / 2) - (button_tex_n.height / 2) - 30, 400, 100);

	if (draw_text_button(batch, "Roll Privately", (width / 2) - (button_tex_n.width / 2), (height / 2) - (button_tex_n.height / 2), FADED_RED, WHITE.xyz)) {
		state = STATE_IDLE;
		i32 num = random_int(1, 6);
		std::string str = "I rolled a dice(1-6): ";
		str.append(std::to_string(num));
		rollLog.push_back(str);
		if (rollLog.size() > 15) {
			rollLog.erase(rollLog.begin());
		}
		//roll to self
	}
	if (draw_text_button(batch, "Roll Publically", (width / 2) - (button_tex_n.width / 2) - 190, (height / 2) - (button_tex_n.height / 2), FADED_RED, WHITE.xyz)) {
		state = STATE_IDLE;
		i32 num = random_int(1, 6);
		std::string str = "I rolled a dice(1-6): ";
		str.append(std::to_string(num));
		rollLog.push_back(str);
		if (rollLog.size() > 15) {
			rollLog.erase(rollLog.begin());
		}

		std::string msg = "roll|1|The DM|";
		msg.append(std::to_string(num));
		msg.append("\n");
		//roll to all clients.
		socket->write_some(boost::asio::buffer(msg, msg.size()));
	}
}