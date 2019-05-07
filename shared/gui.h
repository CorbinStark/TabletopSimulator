#ifndef GUI_H
#define GUI_H

#include <defines.h>
#include <maths.h>
#include "globals.h"
#include "../DnDShared/font.h"
#include "../DnDShared/render2D.h"

enum TextFieldType {
	INPUT_TEXT_ONLY,
	INPUT_NUMBERS_ONLY,
	INPUT_EVERYTHING
};

struct TextField {
	StringList text;
	vec4 bgcolor;
	vec4 fgcolor;
	vec3 textcolor;
	TextFieldType type;
	u16 maxChars;
	u16 maxLines;
	u16 width;
	u16 height;
};

struct Panel {
	TextField* active;
	TextField* hot;
};

INTERNAL inline
void begin_gui(Panel* panel) {
	if (is_button_released(MOUSE_BUTTON_LEFT)) {
		*panel = { 0 };
	}
}

INTERNAL inline
void end_gui(Panel* panel) {

}

INTERNAL inline
std::string get_text(TextField* field) {
	std::string result;
	for (i32 i = 0; i < field->text.size(); ++i) {
		result.append(field->text[i]);
	}
	return result;
}

INTERNAL inline
void draw_text_field(RenderBatch* batch, Panel* panel, Font& font, TextField* field, const u16 xPos, const u16 yPos) {
	vec2 mousepos = get_mouse_pos();
	Rect fieldrect = rect(xPos, yPos, field->width, field->height);
	if (is_button_released(MOUSE_BUTTON_LEFT)) {
		if (colliding(fieldrect, mousepos.x, mousepos.y)) {
			if (panel->hot != field) {
				panel->hot = field;
			}
			panel->active = field;
		}
	}
	draw_rectangle(batch, xPos, yPos, field->width, field->height, field->bgcolor);
	draw_rectangle(batch, xPos + 3, yPos + 3, field->width - 6, field->height - 6, field->fgcolor);
	
	const int padding = 20;

	if (panel->active == field) {
		if ((i32)(get_elapsed_time() * 100) % 100 > 50)
			for(int i = 0; i < field->text.size(); ++i)
				draw_text(batch, &font, field->text[i], xPos + 3, yPos + 3 + (i * padding), field->textcolor.x, field->textcolor.y, field->textcolor.z);
		else if (panel->active == field) {
			for(int i = 0; i < field->text.size()-1; ++i)
				draw_text(batch, &font, field->text[i], xPos + 3, yPos + 3 + (i * padding), field->textcolor.x, field->textcolor.y, field->textcolor.z);
			draw_text(batch, &font, field->text[field->text.size()-1], xPos + 3, yPos + 3 + ((field->text.size()-1) * padding), field->textcolor.x, field->textcolor.y, field->textcolor.z);
			draw_text(batch, &font, "|", xPos + 3 + get_string_width(font, field->text[field->text.size() - 1].c_str()) - 5, yPos + 3 + ((field->text.size() - 1) * padding), field->textcolor.x, field->textcolor.y, field->textcolor.z);
		}
	}
	else {
		for(int i = 0; i < field->text.size(); ++i)
			draw_text(batch, &font, field->text[i], xPos + 3, yPos + 3 + (i * padding), field->textcolor.x, field->textcolor.y, field->textcolor.z);
	}

	if (panel->active == field) {
		i32 key = get_key_pressed();
		if (is_key_pressed(KEY_BACKSPACE) && !field->text.empty()) {
			if (field->text[field->text.size() - 1].size() == 0 && field->text.size() > 1) {
				field->text.pop_back();
			}
			if (field->text.size() > 0 && field->text[field->text.size() > 1 ? field->text.size() - 1 : 0].size() > 0) {
				field->text[field->text.size() - 1].pop_back();
			}
		}
		else if (key == KEY_SPACE)
			field->text[field->text.size()-1].push_back(' ');
		else if (key != 0 && key != KEY_ENTER) {
			i32 numChars = 0;
			for (i32 i = 0; i < field->text.size(); ++i)
				numChars += field->text.at(i).size();

			if (field->type == INPUT_EVERYTHING) {
				if (numChars < field->maxChars && key >= '!' && key <= '~') {
					field->text[field->text.size() - 1].push_back(key);
				}
			}
			if (field->type == INPUT_NUMBERS_ONLY) {
				if (numChars < field->maxChars && key >= '0' && key <= '9') {
					field->text[field->text.size() - 1].push_back(key);
				}
			}
		}
	}
	if (field->text.size() < field->maxLines) {
		if (get_string_width(font, field->text[field->text.size() - 1].c_str()) > field->width - 15)
			field->text.push_back("");
		if (panel->active == field && is_key_pressed(KEY_ENTER))
			field->text.push_back("");
	}
}

INTERNAL inline
bool draw_text_button(RenderBatch* batch, const char* const text, const u32 text_width, const u16 xPos, const u16 yPos, const vec4 color, vec3 textColor) {
	i32 oldwidth = button_tex_n.width;
	if (text_width > button_tex_n.width) {
		button_tex_n.width = text_width + 8;
		button_tex_h.width = text_width + 8;
		button_tex_p.width = text_width + 8;
	}

	const Rect button = rect(xPos, yPos, button_tex_n.width, button_tex_n.height);
	const vec2 mouse_pos = get_mouse_pos();
	const bool collided = colliding(button, mouse_pos.x, mouse_pos.y);
	const bool buttonReleased = is_button_released(MOUSE_BUTTON_LEFT);
	i8 yOffset = -2;

	if (collided) {
		if (is_button_down(MOUSE_BUTTON_LEFT)) {
			draw_texture(batch, button_tex_p, xPos, yPos, color.x, color.y, color.z, color.w);
			yOffset = 0;
		}
		else
			draw_texture(batch, button_tex_h, xPos, yPos, color.x, color.y, color.z, color.w);
	}
	else
		draw_texture(batch, button_tex_n, xPos, yPos, color.x, color.y, color.z, color.w); //highlight if mouse is on button

	draw_text(batch, &HEADER_FONT, text, xPos + (button_tex_n.width / 2) - (text_width / 2),
		yPos + yOffset + (button_tex_n.height / 2) - (HEADER_FONT.characters['P']->texture.height / 2),
		textColor.x, textColor.y, textColor.z
	);

	button_tex_n.width = oldwidth;
	button_tex_h.width = oldwidth;
	button_tex_p.width = oldwidth;

	return collided & buttonReleased;
}

//returns true if clicked
INTERNAL inline
bool draw_text_button(RenderBatch* batch, const char* const text, const u16 xPos, const u16 yPos, vec4 color, vec3 textColor) {
	const u32 text_width = get_string_width(HEADER_FONT, text);
	return draw_text_button(batch, text, text_width, xPos, yPos, color, textColor);
}

#endif