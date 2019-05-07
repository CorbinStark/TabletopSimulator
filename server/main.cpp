#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <fstream>

#include "../DnDShared/globals.h"
#include "../DnDShared/font.h"
#include "../DnDShared/gui.h"

#include "map.h"

#include "accounts.h"
#include "networking.h"

using namespace boost::asio;
using namespace boost::asio::ip;

INTERNAL StringList rollLog;
INTERNAL Map map;
INTERNAL boost::mutex mutex;

INTERNAL void receive(Server* server, Socket* sender, StringList* message);
INTERNAL void draw_usernames(RenderBatch* batch, Server* server);
INTERNAL void map_input(Map* map);
INTERNAL void draw_log(RenderBatch* batch);
INTERNAL void roll_prompt(RenderBatch* batch, Server* server, GameState& state);

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
	draw_rectangle(batch, x+5, y+5, width - 10, height - 10, WHITE);
}

INTERNAL inline
void set_bar_value(StatusBar* bar, TextField* field1, TextField* field2) {
	if (field1->text.size() > 0 && field1->text[0] != "")
		bar->current = std::stoi(field1->text[0]);
	if (field2->text.size() > 0 && field2->text[0] != "")
		bar->max = std::stoi(field2->text[0]);
}

int main() {
	Server server;
	set_receive_callback(&server, receive);
	start_server(&server);

	init_window(1400, 800, "Jojo Tabletop DM Console", false, true, true);
	init_audio();
	set_fps_cap(60);
	set_master_volume(100);
	set_clear_color(FADED_TEAL);
	set_mouse_state(MOUSE_HIDDEN);

	RenderBatch* batch = &create_batch();

	map = { 0 };
	map.selected = -1;
	map.width = map.height = 20;
	map.bgColor = WHITE;
	map.gridColor = GRAY;
	Token token = { 0 };
	token.imgindex = 2;
	token.xPos = token.yPos = 128;
	map.tokens.push_back(token);
	token = { 0 };
	token.imgindex = 1;
	token.xPos = token.yPos = 256;
	map.tokens.push_back(token);

	Shader basic = load_default_shader_2D();
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
	TextField bizarrePoints = create_textfield(100, 30, 6, 1, INPUT_NUMBERS_ONLY);
	TextField charTypes = create_textfield(490, 30, 30, 1, INPUT_EVERYTHING);
	TextField charBackstory = create_textfield(400, 200, 300, 20, INPUT_EVERYTHING);
	TextField inventory = create_textfield(400, 200, 22, 1, INPUT_EVERYTHING);

	//textfields for stand sheet
	TextField standName = create_textfield(450, 30, 22, 1, INPUT_EVERYTHING);
	TextField standTypes = create_textfield(450, 30, 22, 1, INPUT_EVERYTHING);
	TextField standDesc = create_textfield(500, 400, 700, 100, INPUT_EVERYTHING);

	f64 zoom = .75;
	while (window_open()) {
		vec2 mousePos = get_mouse_pos();
		zoom += get_scroll_y() * 0.015625f;
		map_input(&map);

		f32 width  = (f32)get_window_width();
		f32 height = (f32)get_window_height();

		mat4 ortho = orthographic_projection(0, 0, width, height, -1, 1);
		begin_drawing();
		//draw stuff that scales with zoom
		begin2D(batch, basic);
			set_viewport(0, 0, width, height);
			upload_mat4(basic, "projection", ortho * scale(zoom, zoom, 1));
			draw_map(batch, &map, zoom);
			if(state == STATE_IDLE)
				update_map(batch, &map, &server, state, zoom);
			draw_tokens(batch, &map, zoom);
		end2D(batch);

		//draw stuff that doesnt scale with zoom
		begin2D(batch, basic);
		begin_gui(&panel);

			upload_mat4(basic, "projection", ortho);
			draw_usernames(batch, &server);
			if(state != STATE_CHARSHEET && state != STATE_STANDSHEET) {
				draw_log(batch);
				//draw buttons
				if (draw_text_button(batch, "Roll Dice", width - (250 / 2) - (button_tex_n.width / 1.5), height - 50, FADED_RED, WHITE.xyz)) {
					state = STATE_ROLL_PROMPT;
				}
			}
			if (state == STATE_SQUARE_SELECT) {
				draw_rectangle(batch, 52, 32, 200, 200, GRAY);
				draw_rectangle(batch, 54, 34, 196, 196, WHITE);
				draw_text(batch, &BODY_FONT, "Background Color", 56, 38, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				for (u8 i = 0; i < NUM_COLORS; ++i) {
					draw_rectangle(batch, 56 + (i * 16), 58, 15, 15, colors[i]);
				}
				//draw_text(BODY_FONT, "Foreground Color", 56, 38, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
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
						if(imageNum > 0 && imageNum < TOKEN_IMAGE_COUNT)
							map.tokens[map.selected].imgindex = imageNum;
					}
					map.tokens[map.selected].name = nameField.text[0];
					send_packet_all(&server, format_text("update_token|%d|%d|%d|%d|%d|%d|%d|%s|%d\n",
						map.selected, current->bar1.current, current->bar1.max, current->bar2.current, current->bar2.max,
						current->bar3.current, current->bar3.max, current->name.c_str(), current->imgindex)
					);
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
				draw_text_field(batch, &panel, font, &charBackstory, xPos + 15, yPos += 30);
				draw_text_field(batch, &panel, font, &occupation, xPos + 430, yPos);
				draw_text_field(batch, &panel, font, &nationality, xPos + 430, yPos + 70);
				draw_text_field(batch, &panel, font, &brains, xPos + 430, yPos + 140);
				draw_text_field(batch, &panel, font, &brawns, xPos + 550, yPos + 140);
				draw_text_field(batch, &panel, font, &bravery, xPos + 550, yPos + 210);
				draw_text_field(batch, &panel, font, &totalHealth, xPos + 430, yPos + 210);
				draw_text_field(batch, &panel, font, &currentHealth, xPos + 430, yPos + 280);
				draw_text_field(batch, &panel, font, &resolveDamage, xPos + 430, yPos + 345);

				draw_text(batch, &font, "Inventory", xPos + 15, yPos += 205, DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
				draw_text_field(batch, &panel, font, &inventory, xPos + 15, yPos += 30);

				if (draw_text_button(batch, "Change Char Sheets", xPos + 285, yPos += 230, FADED_RED, WHITE.xyz)) {
					state = STATE_STANDSHEET;
				}
				if (draw_text_button(batch, "Save Changes", xPos + 15, yPos, FADED_RED, WHITE.xyz)) {

					state = STATE_IDLE;
				}
				if (draw_text_button(batch, "Cancel", xPos + 185, yPos, FADED_RED, WHITE.xyz)) {
					state = STATE_IDLE;
				}
			}
			//draw sidebar
			if (state != STATE_TOKEN) {
				i32 yPos = -30;
				if (draw_icon_button(batch, &select_button, 10, yPos += 34, 1)) {
					state = STATE_IDLE;
				}
				if (draw_icon_button(batch, &square_button, 10, yPos += 34, 1)) {
					state = STATE_SQUARE_SELECT;
				}
				if (draw_icon_button(batch, &circle_button, 10, yPos += 34, 1)) {
					state = STATE_CIRCLE;
				}
				if (draw_icon_button(batch, &turn_button, 10, yPos += 34, 1)) {
					send_packet_all(&server, "turncounter\n");
				}
				if (draw_icon_button(batch, &layers_button, 10, yPos += 34, 1)) {

				}
				if (draw_icon_button(batch, &roundabout_button, 10, yPos += 34, 1)) {
					send_packet_all(&server, "roundabout\nplay_music|1\n");
					play_sound(music[1]);
				}
				if (draw_icon_button(batch, &battle_music_button, 10, yPos += 34, 1)) {
					send_packet_all(&server, "play_music|0\n");
					play_sound(music[0]);
				}
				if (draw_icon_button(batch, &menacing_button, 10, yPos += 34, 1)) {
					send_packet_all(&server, "menacing\n");
				}
			}
			if (state == STATE_ROLL_PROMPT) {
				roll_prompt(batch, &server, state);
			}

			draw_texture(batch, cursor, (i32)mousePos.x, (i32)mousePos.y);

		end_gui(&panel);
		end2D(batch);
		end_drawing();
	}
	stop_server(&server);
	dispose_window();

	return 0;
}

//receive messages from clients (these are automatically sent back to other connected clients as well)
INTERNAL
void receive(Server* server, Socket* sender, StringList* message) {
	if (message->at(0) == "roll") {
		std::string str;
		str.append(message->at(2));
		str.append(" rolled a dice(1-6): ");
		str.append(message->at(3));
		str.pop_back();
		rollLog.push_back(str);
		if (rollLog.size() > 15) {
			rollLog.erase(rollLog.begin());
		}
	}
	if (message->at(0) == "move") {
		map.tokens[std::atoi( message->at(1).c_str() )].xPos = std::atoi( message->at(2).c_str() );
		map.tokens[std::atoi( message->at(1).c_str() )].yPos = std::atoi( message->at(3).c_str() );
	}
	if (message->at(0) == "update_token") {
		i32 ndx = std::stoi(message->at(1));
		if (ndx == -1) {
			Token token = { 0 };
			map.tokens.push_back(token);
			ndx = map.tokens.size() - 1;
		}
		map.tokens[ndx].bar1.current = std::stoi(message->at(2));
		map.tokens[ndx].bar1.max = std::stoi(message->at(3));
		map.tokens[ndx].bar2.current = std::stoi(message->at(4));
		map.tokens[ndx].bar2.max = std::stoi(message->at(5));
		map.tokens[ndx].bar3.current = std::stoi(message->at(6));
		map.tokens[ndx].bar3.max = std::stoi(message->at(7));
		map.tokens[ndx].name = message->at(8);
		map.tokens[ndx].imgindex = std::stoi(message->at(9));
	}
	if (message->at(0) == "name") {
		//format: update_map|width|height|xPos|yPos|grid|fow|bgColor|gridColor|selected\n
		send_packet_all(server, format_text("update_map|%d|%d|%d|%d|%d|%d|%f|%f|%f|%f|%f|%f|%f|%f|%d\n",
			map.width, map.height, map.xPos, map.yPos, map.grid, map.fow, map.bgColor.x, map.bgColor.y, map.bgColor.z, map.bgColor.w,
			map.gridColor.x, map.gridColor.y, map.gridColor.z, map.gridColor.w, -1)
		);

		for (int i = 0; i < map.tokens.size(); ++i) {
			Token* current = &map.tokens[i];
			std::string command = format_text("update_token|-1|%d|%d|%d|%d|%d|%d|%s|%d\n",
				current->bar1.current, current->bar1.max, current->bar2.current, current->bar2.max,
				current->bar3.current, current->bar3.max, current->name.c_str(), current->imgindex
			);
			send_packet_all(server, command);
		}
	}
	if (message->at(0) == "update_map") {
		map.tokens.clear();
		map.rects.clear();
		map = { 0 };

		map.width = std::stoi(message->at(1));
		map.height = std::stoi(message->at(2));
		map.xPos = std::stoi(message->at(3));
		map.yPos = std::stoi(message->at(4));
		map.grid = std::stoi(message->at(5));
		map.fow = std::stoi(message->at(6));
		map.bgColor = { std::stof(message->at(7)), std::stof(message->at(8)), std::stof(message->at(9)), std::stof(message->at(10)) };
		map.gridColor = { std::stof(message->at(11)), std::stof(message->at(12)), std::stof(message->at(13)), std::stof(message->at(14)) };
		map.selected = std::stoi(message->at(15));
	}
	if (message->at(0) == "update_account") {
		server->userListMutex.lock();
		for (u32 i = 0; i < server->users.size(); ++i) {
			if (message->at(1) == server->users.at(i).name) {
				message->erase(message->begin());
				Account acc;
				acc.name = message->at(0);
				acc.pass = message->at(1);
				read_stand_charsheet(&acc.standsheet, message);
				read_user_charsheet(&acc.usersheet, message);
				write_account_data(&acc);
				break;
			}
		}
		server->userListMutex.unlock();
	}
}

//draw the names of the connected players at the bottom of the screen
INTERNAL
void draw_usernames(RenderBatch* batch, Server* server) {
	server->userListMutex.lock();
	for (int i = 0; i < server->users.size(); ++i) {
		i32 x = (i * 110) + 20;
		i32 y = get_window_height() - 40;
		i32 width = get_string_width(BODY_FONT, server->users[i].name.c_str()) + 8;
		i32 height = 30;

		draw_rectangle(batch, x, y, width, height, FADED_RED);
		draw_text(batch, &BODY_FONT, server->users[i].name, x + 5, y + (height / 2) - (BODY_FONT.characters['t']->texture.height / 2), 255, 255, 255);
	}
	server->userListMutex.unlock();
}

//decide whether to roll privately or publically
INTERNAL
void roll_prompt(RenderBatch* batch, Server* server, GameState& state) {
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
		send_packet_all(server, msg);
	}
}

INTERNAL 
void draw_log(RenderBatch* batch) {
	draw_outline(batch, get_window_width() - 255, 0, 255, get_window_height());

	for (u16 i = 0; i < rollLog.size(); ++i) {
		draw_text(batch, &BODY_FONT, rollLog[i], get_window_width() - 240, 10 + (i * 25), DARKGRAY.x, DARKGRAY.y, DARKGRAY.z);
	}
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