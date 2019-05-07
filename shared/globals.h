#ifndef GLOBALS_H
#define GLOBALS_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/algorithm/string.hpp>
#include <queue>
#include <vector>
#include "bahamut.h"

#define BUFFER_SIZE 1024
#define SHORT_SLEEP 100
#define LONG_SLEEP  400
#define PROTOCOL    tcp

typedef boost::asio::ip::PROTOCOL::socket	Socket;
typedef boost::shared_ptr<Socket>			SocketPtr;
typedef boost::shared_ptr<std::string>		StringPtr;

struct ClientMessage {
	Socket* socket;
	std::string str;
};

typedef std::vector<Socket*>		ClientList;
typedef std::queue<ClientMessage>	MessageQueue;
typedef std::queue<std::string>		StringQueue;
typedef std::vector<std::string>	StringList;
#define TEXTURE_PARAM GL_NEAREST
#define SCROLL_SPEED 20
#define TILESIZE 128

static const vec4 FADED_GREEN = V4(117, 184, 154, 255);
static const vec4 FADED_RED   = V4(184, 87, 104, 255);
static const vec4 FADED_WHITE = V4(252, 252, 252, 255);
static const vec4 FADED_TEAL  = V4(176, 211, 212, 255);
static const vec4 FADED_BLUE  = V4(113, 140, 197, 255);
static const vec4 FADED_BLACK = V4(10, 26, 63, 255);

#define TOKEN_IMAGE_COUNT 10

//Textures and fonts
extern Texture cursor;
extern Texture button_tex_n;
extern Texture button_tex_h;
extern Texture button_tex_p;
extern Texture menu_tex[9];
extern Texture tokenimages[TOKEN_IMAGE_COUNT];
extern Texture menacing;
extern Texture to_be_continued;
extern Texture settings_icon;
extern Texture exit_icon;
extern Texture char_sheet_icon;

extern Texture select_button;
extern Texture layers_button;
extern Texture square_button;
extern Texture circle_button;
extern Texture turn_button;
extern Texture battle_music_button;
extern Texture roundabout_button;
extern Texture menacing_button;

extern Sound music[10];
extern Font BODY_FONT;
extern Font HEADER_FONT;

static inline
Texture getSubImage(unsigned char* pixels, int pixels_width, int x, int y, int width, int height) {
	Texture subimage;
	glPixelStorei(GL_UNPACK_ROW_LENGTH, pixels_width);
	unsigned char* subimage_pixels = pixels + (x + y * pixels_width) * 4;
	subimage = load_texture(subimage_pixels, width, height, TEXTURE_PARAM);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	return subimage;
}

static inline
StringList split_string(std::string string, char divider) {
	StringList tokens;

	std::string current;
	for (u32 i = 0; i < string.size(); ++i) {
		char c = string.at(i);
		if (c != divider)
			current.push_back(c);
		if (c == divider || (i + 1) == string.size()) {
			tokens.push_back(current);
			current.clear();
		}
	}

	return tokens;
}

#include <random>
namespace {
	std::random_device rd;
	std::mt19937 mt(rd());

	//================================================
	//Description: Generates a random integer from 0
	//	to a value.
	//================================================
	int inline random_int(int exclusiveMax) {
		std::uniform_int_distribution<> dist(0, exclusiveMax - 1);
		return dist(mt);
	}

	//================================================
	//Description: Generates a random integer between
	//	two values.
	//================================================
	int inline random_int(int min, int max) {
		std::uniform_int_distribution<> dist(0, max - min);
		return dist(mt) + min;
	}
}

#include <SOIL.h>

void load_all_textures();

#endif