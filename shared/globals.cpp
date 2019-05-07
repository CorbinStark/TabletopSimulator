#include "globals.h"

Texture cursor;
Texture button_tex_n;
Texture button_tex_h;
Texture button_tex_p;
Texture menu_tex[9];
Texture tokenimages[10];
Texture menacing;
Texture to_be_continued;
Texture settings_icon;
Texture exit_icon;
Texture char_sheet_icon;

Texture select_button;
Texture layers_button;
Texture square_button;
Texture circle_button;
Texture turn_button;
Texture battle_music_button;
Texture roundabout_button;
Texture menacing_button;

Sound music[10];
Font BODY_FONT;
Font HEADER_FONT;

void load_all_textures() {
	cursor = load_texture("art/cursor.png", TEXTURE_PARAM);
	BODY_FONT = load_font("art/OpenSans-Regular.ttf", 24, GL_LINEAR);
	HEADER_FONT = load_font("art/OpenSans-Regular.ttf", 24, GL_LINEAR);

	button_tex_n = load_texture("art/button_n.png", TEXTURE_PARAM);
	button_tex_h = load_texture("art/button_h.png", TEXTURE_PARAM);
	button_tex_p = load_texture("art/button_p.png", TEXTURE_PARAM);

	tokenimages[0] = load_texture("art/jojo.png", TEXTURE_PARAM);
	tokenimages[1] = load_texture("art/io.png", TEXTURE_PARAM);
	tokenimages[2] = load_texture("art/Dont_ask.png", TEXTURE_PARAM);
	tokenimages[3] = load_texture("art/VigilanteSquare.png", TEXTURE_PARAM);
	tokenimages[4] = load_texture("art/GoldVigilanteSquare.png", TEXTURE_PARAM);
	tokenimages[5] = load_texture("art/jojo.png", TEXTURE_PARAM);
	tokenimages[6] = load_texture("art/jojo.png", TEXTURE_PARAM);
	tokenimages[7] = load_texture("art/jojo.png", TEXTURE_PARAM);
	tokenimages[8] = load_texture("art/jojo.png", TEXTURE_PARAM);
	tokenimages[9] = load_texture("art/jojo.png", TEXTURE_PARAM);

	select_button = load_texture("art/select_button.png", TEXTURE_PARAM);
	layers_button = load_texture("art/layers_button.png", TEXTURE_PARAM);
	square_button = load_texture("art/square_button.png", TEXTURE_PARAM);
	circle_button = load_texture("art/circle_button.png", TEXTURE_PARAM);
	char_sheet_icon = load_texture("art/char_sheet_icon.png", TEXTURE_PARAM);
	turn_button = load_texture("art/turn_button.png", TEXTURE_PARAM);
	roundabout_button = load_texture("art/roundabout_button.png", TEXTURE_PARAM);
	battle_music_button = load_texture("art/battle_music_button.png", TEXTURE_PARAM);
	menacing_button = load_texture("art/menacing_button.png", TEXTURE_PARAM);

	menacing = load_texture("art/menacing.png", TEXTURE_PARAM);
	to_be_continued = load_texture("art/to_be_continued.png", TEXTURE_PARAM);
	settings_icon = load_texture("art/settings.png", TEXTURE_PARAM);
	exit_icon = load_texture("art/exit.png", TEXTURE_PARAM);

	music[0] = load_sound("audio/001.wav");
	music[1] = load_sound("audio/002.wav");
	music[2] = load_sound("audio/001.wav");
	music[3] = load_sound("audio/001.wav");
	music[4] = load_sound("audio/001.wav");
	music[5] = load_sound("audio/001.wav");
	music[6] = load_sound("audio/001.wav");
	music[7] = load_sound("audio/001.wav");
	music[8] = load_sound("audio/001.wav");
	music[9] = load_sound("audio/001.wav");
	int w;
	int h;
	int i = 0;
	unsigned char* image = SOIL_load_image("art/menu.png", &w, &h, 0, SOIL_LOAD_RGBA);
	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
			menu_tex[i++] = getSubImage(image, w, x * (w / 3), y * (h / 3), w / 3, h / 3);
	free(image);
}