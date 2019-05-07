///////////////////////////////////////////////////////////////////////////
// FILE:                        font.h                                   //
///////////////////////////////////////////////////////////////////////////
//                      BAHAMUT GRAPHICS LIBRARY                         //
//                        Author: Corbin Stark                           //
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Corbin Stark                                       //
//                                                                       //
// Permission is hereby granted, free of charge, to any person obtaining //
// a copy of this software and associated documentation files (the       //
// "Software"), to deal in the Software without restriction, including   //
// without limitation the rights to use, copy, modify, merge, publish,   //
// distribute, sublicense, and/or sell copies of the Software, and to    //
// permit persons to whom the Software is furnished to do so, subject to //
// the following conditions:                                             //
//                                                                       //
// The above copyright notice and this permission notice shall be        //
// included in all copies or substantial portions of the Software.       //
//                                                                       //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       //
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    //
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.//
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  //
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  //
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     //
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                //
///////////////////////////////////////////////////////////////////////////

#ifndef FONT_H
#define FONT_H

#include "defines.h"
#include "texture.h"
#include "maths.h"
#include <ft2build.h>
#include FT_FREETYPE_H 

struct Character {
	Texture	texture;
	vec2	size;
	vec2	bearing;
	GLuint	advance;
	};

struct Font {
	Character* characters[128];
	FT_Face face;
	FT_Library ft;
	int size;
};

Texture create_texture_from_string(Font& font, const std::string str, i32 texParam);
Texture create_texture_from_string(Font& font, const std::string str, i32 texParam, GLubyte r, GLubyte g, GLubyte b);
Font load_font(const GLchar* filepath, unsigned int size, i32 texParam);
void dispose_font(Font& font);

float get_font_height(Font& font);

inline const u32 get_string_width(Font& font, const char* str) {
	u32 len = strlen(str);
	u32 width = 0;
	for (u32 i = 0; i < len; ++i) {
		if (str[i] == '\n') return width;
		width += (font.characters[str[i]]->advance >> 6);
	}
	return width;
}

//text must be less than 128 chars long.
inline const char* format_text(const char* text, ...) {
	static char buffer[MAX_FORMAT_TEXT_SIZE];

	va_list args;
	va_start(args, text);
	vsprintf(buffer, text, args);
	va_end(args);

	return buffer;
}
#endif