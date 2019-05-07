///////////////////////////////////////////////////////////////////////////
// FILE:                       render2D.h                                //
///////////////////////////////////////////////////////////////////////////
//                      BAHAMUT GRAPHICS LIBRARY                         //
//                        Author: Corbin Stark                           //
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Corbin Stark                                       //
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

#ifndef RENDER2D_H
#define RENDER2D_H

#include "defines.h"
#include "shader.h"
#include "texture.h"
#include "font.h"

struct VertexData {
	vec2 pos;
	vec4 color; //32 bit color (8 for R, 8 for G, 8 for B, 8 for A)
	vec2 uv;
	f32 texid;
};

#ifndef BATCH_MAX_SPRITES
#define BATCH_MAX_SPRITES	    20000
#endif

#define BATCH_VERTEX_SIZE	    sizeof(VertexData)
#define BATCH_SPRITE_SIZE	    BATCH_VERTEX_SIZE * 4
#define BATCH_BUFFER_SIZE	    BATCH_SPRITE_SIZE * BATCH_MAX_SPRITES
#define BATCH_INDICE_SIZE	    BATCH_MAX_SPRITES * 6
#define BATCH_MAX_TEXTURES		16

struct RenderBatch {
	u32 vao;
	u32 vbo;
	u32 ebo;
	u16 indexcount;
	u16 texcount;
	GLuint  textures[BATCH_MAX_TEXTURES];
	GLchar* locations[BATCH_MAX_TEXTURES];
	VertexData* buffer;
	Shader shader;
};

void end2D(RenderBatch* batch);

INTERNAL inline
RenderBatch create_batch() {
	RenderBatch batch = { 0 };

	for (u32 i = 0; i < BATCH_MAX_TEXTURES; ++i)
		batch.locations[i] = (GLchar*)malloc(6 * sizeof(GLchar));

	strcpy(batch.locations[0], "tex1");
	strcpy(batch.locations[1], "tex2");
	strcpy(batch.locations[2], "tex3");
	strcpy(batch.locations[3], "tex4");
	strcpy(batch.locations[4], "tex5");
	strcpy(batch.locations[5], "tex6");
	strcpy(batch.locations[6], "tex7");
	strcpy(batch.locations[7], "tex8");
	strcpy(batch.locations[8], "tex9");
	strcpy(batch.locations[9], "tex10");
	strcpy(batch.locations[10], "tex11");
	strcpy(batch.locations[11], "tex12");
	strcpy(batch.locations[12], "tex13");
	strcpy(batch.locations[13], "tex14");
	strcpy(batch.locations[14], "tex15");
	strcpy(batch.locations[15], "tex16");

	glGenVertexArrays(1, &batch.vao);
	glBindVertexArray(batch.vao);

	glGenBuffers(1, &batch.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, batch.vbo);
	glBufferData(GL_ARRAY_BUFFER, BATCH_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);

	//the last argument to glVertexAttribPointer is the offset from the start of the vertex to the
	//data you want to look at - so each new attrib adds up all the ones before it.
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, BATCH_VERTEX_SIZE, (const GLvoid*)0);                     //vertices
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, BATCH_VERTEX_SIZE, (const GLvoid*)(2 * sizeof(GLfloat))); //color
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, BATCH_VERTEX_SIZE, (const GLvoid*)(6 * sizeof(GLfloat))); //tex coords
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, BATCH_VERTEX_SIZE, (const GLvoid*)(8 * sizeof(GLfloat))); //texture id

	GLushort indices[BATCH_INDICE_SIZE];

	int offset = 0;
	for (u32 i = 0; i < BATCH_INDICE_SIZE; i += 6) {
		indices[i] = offset + 0;
		indices[i + 1] = offset + 1;
		indices[i + 2] = offset + 2;
		indices[i + 3] = offset + 2;
		indices[i + 4] = offset + 3;
		indices[i + 5] = offset + 0;

		offset += 4;
	}

	glGenBuffers(1, &batch.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, BATCH_INDICE_SIZE * sizeof(GLushort), indices, GL_STATIC_DRAW);

	//the vao must be unbound before the buffers
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return batch;
}

INTERNAL inline
void begin2D(RenderBatch* batch, Shader shader, bool blending = true, bool depthTest = false) {
	batch->shader = shader;
	start_shader(shader);

	if (blending)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
	if (depthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	glBindBuffer(GL_ARRAY_BUFFER, batch->vbo);
	batch->buffer = (VertexData*)glMapBufferRange(GL_ARRAY_BUFFER, 0, BATCH_BUFFER_SIZE,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
	);
}

INTERNAL inline
i32 submit_tex(RenderBatch* batch, Texture tex) {
	int texSlot = 0;
	bool found = false;
	for (u32 i = 0; i < batch->texcount; ++i) {
		if (batch->textures[i] == tex.ID) {
			texSlot = (i + 1);
			found = true;
			break;
		}
	}
	if (!found) {
		if (batch->texcount >= BATCH_MAX_TEXTURES) {
			end2D(batch);
			begin2D(batch, batch->shader);
		}
		batch->textures[batch->texcount++] = tex.ID;
		texSlot = batch->texcount;
	}
	return texSlot;
}

INTERNAL inline
void draw_texture(RenderBatch* batch, Texture tex, i32 xPos, i32 yPos, f32 r, f32 g, f32 b, f32 a) {
	if (tex.ID == 0)
		return;

	r /= 255;
	g /= 255;
	b /= 255;
	a /= 255;

	i32 texSlot = submit_tex(batch, tex);

	f32 x = (f32)xPos;
	f32 y = (f32)yPos;

	LOCAL f32 DEFAULT_UVS[8] = {0, 0, 0, 1, 1, 1, 1, 0};
	LOCAL f32 FLIP_HOR_UVS[8] = {1, 1, 1, 0, 0, 0, 0, 1};
	LOCAL f32 FLIP_VER_UVS[8] = {0, 1, 0, 0, 1, 0, 1, 1};
	LOCAL f32 FLIP_BOTH_UVS[8] = {1, 0, 1, 1, 0, 1, 0, 0};

	GLfloat* uvs;
	uvs = DEFAULT_UVS;
	if (tex.flip_flag & FLIP_HORIZONTAL && tex.flip_flag & FLIP_VERTICAL)
		uvs = FLIP_BOTH_UVS;
	if (tex.flip_flag & FLIP_HORIZONTAL)
		uvs = FLIP_HOR_UVS;
	if (tex.flip_flag & FLIP_VERTICAL)
		uvs = FLIP_VER_UVS;

	batch->buffer->pos = {x, y};
	batch->buffer->color = {r, g, b, a};
	batch->buffer->uv = {uvs[0], uvs[1]};
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->buffer->pos = {x, y + tex.height};
	batch->buffer->color = {r, g, b, a};
	batch->buffer->uv = {uvs[2], uvs[3]};
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->buffer->pos = {x + tex.width, y + tex.height};
	batch->buffer->color = {r, g, b, a};
	batch->buffer->uv = {uvs[4], uvs[5]};
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->buffer->pos = {x + tex.width, y};
	batch->buffer->color = {r, g, b, a};
	batch->buffer->uv = {uvs[6], uvs[7]};
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->indexcount += 6;
}

INTERNAL inline
void draw_texture(RenderBatch* batch, Texture tex, i32 x, i32 y) {
	draw_texture(batch, tex, x, y, 255, 255, 255, 255);
}

INTERNAL inline
void draw_texture(RenderBatch* batch, Texture tex, i32 x, i32 y, vec4 rgba) {
	draw_texture(batch, tex, x, y, rgba.x, rgba.y, rgba.z, rgba.w);
}

INTERNAL inline
void draw_texture_rotated(RenderBatch* batch, Texture tex, i32 x, i32 y, vec2 origin, f32 rotation, f32 r, f32 g, f32 b, f32 a) {
	if (tex.ID == 0)
		return;
	i32 texSlot = submit_tex(batch, tex);

	LOCAL f32 FLIP_VER_UVS[8] = { 0, 1, 0, 0, 1, 0, 1, 1 };
	LOCAL f32 FLIP_HOR_UVS[8] = { 1, 1, 1, 0, 0, 0, 0, 1 };
	LOCAL f32 DEFAULT_UVS[8] = { 0, 0, 0, 1, 1, 1, 1, 0 };
	LOCAL f32 FLIP_BOTH_UVS[8] = { 1, 0, 1, 1, 0, 1, 0, 0 };

	GLfloat* uvs;
	uvs = DEFAULT_UVS;
	if (tex.flip_flag & FLIP_HORIZONTAL && tex.flip_flag & FLIP_VERTICAL)
		uvs = FLIP_BOTH_UVS;
	if (tex.flip_flag & FLIP_HORIZONTAL)
		uvs = FLIP_HOR_UVS;
	if (tex.flip_flag & FLIP_VERTICAL)
		uvs = FLIP_VER_UVS;

	f32 cosine = 1;
	f32 sine = 0;
	if (rotation != 0) {
		rotation = deg_to_rad(rotation);
		cosine = cos(rotation);
		sine = sin(rotation);
	}

	batch->buffer->pos = {
		cosine * (x - origin.x) - sine * (y - origin.y) + origin.x, 
		sine * (x - origin.x) + cosine * (y - origin.y) + origin.y
	};
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { uvs[0], uvs[1] };
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->buffer->pos = {
		cosine * (x - origin.x) - sine * ((y + tex.height) - origin.y) + origin.x,
		sine * (x - origin.x) + cosine * ((y + tex.height) - origin.y) + origin.y
	};
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { uvs[2], uvs[3] };
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->buffer->pos = {
		cosine * ((x + tex.width) - origin.x) - sine * ((y + tex.height) - origin.y) + origin.x,
		sine * ((x + tex.width) - origin.x) + cosine * ((y + tex.height) - origin.y) + origin.y
	};
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { uvs[4], uvs[5] };
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->buffer->pos = {
		cosine * ((x + tex.width) - origin.x) - sine * (y - origin.y) + origin.x,
		sine * ((x + tex.width) - origin.x) + cosine * (y - origin.y) + origin.y
	};
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { uvs[6], uvs[7] };
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->indexcount += 6;
}

INTERNAL inline
void draw_texture_rotated(RenderBatch* batch, Texture tex, i32 x, i32 y, f32 rotateDegree, f32 r, f32 g, f32 b, f32 a) {
	draw_texture_rotated(batch, tex, x, y, V2(x + (tex.width / 2.0f), y + (tex.height / 2.0f)), rotateDegree, r, g, b, a);
}

INTERNAL inline
void draw_texture_rotated(RenderBatch* batch, Texture tex, i32 x, i32 y, f32 rotateDegree) {
	draw_texture_rotated(batch, tex, x, y, V2(x + (tex.width / 2.0f), y + (tex.height / 2.0f)), rotateDegree, 1, 1, 1, 1);
}

INTERNAL inline
void draw_texture_EX(RenderBatch* batch, Texture tex, Rect source, Rect dest, f32 r, f32 g, f32 b, f32 a) {
	if (tex.ID == 0)
		return;

	r /= 255.0f;
	g /= 255.0f;
	b /= 255.0f;
	a /= 255.0f;

	f32 uvs[8];
	if (tex.flip_flag == 0) {
		uvs[0] = source.x / tex.width;
		uvs[1] = source.y / tex.height;
		uvs[2] = source.x / tex.width;
		uvs[3] = (source.y + source.height) / tex.height;
		uvs[4] = (source.x + source.width) / tex.width;
		uvs[5] = (source.y + source.height) / tex.height;
		uvs[6] = (source.x + source.width) / tex.width;
		uvs[7] = source.y / tex.height;
	}
	else if (tex.flip_flag & FLIP_HORIZONTAL) {
		uvs[0] = (source.x + source.width) / tex.width;
		uvs[1] = source.y / tex.height;
		uvs[2] = (source.x + source.width) / tex.width;
		uvs[3] = (source.y + source.height) / tex.height;
		uvs[4] = source.x / tex.width;
		uvs[5] = (source.y + source.height) / tex.height;
		uvs[6] = source.x / tex.width;
		uvs[7] = source.y / tex.height;
	}
	else if (tex.flip_flag & FLIP_VERTICAL) {
		uvs[0] = source.x / tex.width;
		uvs[1] = (source.y + source.height) / tex.height;
		uvs[2] = source.x / tex.width;
		uvs[3] = source.y / tex.height;
		uvs[4] = (source.x + source.width) / tex.width;
		uvs[5] = source.y / tex.height;
		uvs[6] = (source.x + source.width) / tex.width;
		uvs[7] = (source.y + source.height) / tex.height;
	}
	else if (tex.flip_flag & FLIP_HORIZONTAL && tex.flip_flag & FLIP_VERTICAL) {
		uvs[0] = (source.x + source.width) / tex.width;
		uvs[1] = (source.y + source.height) / tex.height;
		uvs[2] = (source.x + source.width) / tex.width;
		uvs[3] = source.y / tex.height;
		uvs[4] = source.x / tex.width;
		uvs[5] = source.y / tex.height;
		uvs[6] = source.x / tex.width;
		uvs[7] = (source.y + source.height) / tex.height;
	}

	i32 texSlot = submit_tex(batch, tex);

	batch->buffer->pos = {dest.x, dest.y};
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { uvs[0], uvs[1] };
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->buffer->pos = {dest.x, dest.y + dest.height};
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { uvs[2], uvs[3] };
	batch->buffer->texid = texSlot;
	batch->buffer++;
	
	batch->buffer->pos = {dest.x + dest.width, dest.y + dest.height};
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { uvs[4], uvs[5] };
	batch->buffer->texid = texSlot;
	batch->buffer++;
	
	batch->buffer->pos = {dest.x + dest.width, dest.y};
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { uvs[6], uvs[7] };
	batch->buffer->texid = texSlot;
	batch->buffer++;

	batch->indexcount += 6;
}

INTERNAL inline
void draw_texture_EX(RenderBatch* batch, Texture tex, Rect source, Rect dest) {
	draw_texture_EX(batch, tex, source, dest, 255.0f, 255.0f, 255.0f, 255.0f);
}

INTERNAL inline
void draw_texture_EX(RenderBatch* batch, Texture tex, Rect source, Rect dest, vec4 color) {
	draw_texture_EX(batch, tex, source, dest, color.x, color.y, color.z, color.w);
}

INTERNAL inline
void draw_framebuffer(RenderBatch* batch, Framebuffer buffer, i32 x, i32 y) {
	draw_texture(batch, buffer.texture, x, y);
}

INTERNAL inline
void draw_text(RenderBatch* batch, Font* font, const char* str, i32 xPos, i32 yPos, f32 r, f32 g, f32 b) {
	r /= 255;
	g /= 255;
	b /= 255;

	LOCAL f32 FLIP_VER_UVS[8] = { 0, 1, 0, 0, 1, 0, 1, 1 };
	LOCAL f32 FLIP_HOR_UVS[8] = { 1, 1, 1, 0, 0, 0, 0, 1 };
	LOCAL f32 DEFAULT_UVS[8] = { 0, 0, 0, 1, 1, 1, 1, 0 };
	LOCAL f32 FLIP_BOTH_UVS[8] = { 1, 0, 1, 1, 0, 1, 0, 0 };

	u32 len = strlen(str);
	for (u32 i = 0; i < len; ++i) {
		Character* c = font->characters[str[i]];
		int yOffset = (font->characters['T']->bearing.y - c->bearing.y) + 1;
		if (yOffset < 0) yOffset = 0;

		int x = xPos + c->bearing.x;
		int y = yPos + yOffset;

		Texture tex = font->characters[str[i]]->texture;
		int texSlot = submit_tex(batch, tex);
		GLfloat* uvs;
		uvs = DEFAULT_UVS;

		batch->buffer->pos.x = x;
		batch->buffer->pos.y = y;
		batch->buffer->color.x = r;
		batch->buffer->color.y = g;
		batch->buffer->color.z = b;
		batch->buffer->color.w = 1;
		batch->buffer->uv.x = uvs[0];
		batch->buffer->uv.y = uvs[1];
		batch->buffer->texid = texSlot;
		batch->buffer++;

		batch->buffer->pos.x = x;
		batch->buffer->pos.y = y + tex.height;
		batch->buffer->color.x = r;
		batch->buffer->color.y = g;
		batch->buffer->color.z = b;
		batch->buffer->color.w = 1;
		batch->buffer->uv.x = uvs[2];
		batch->buffer->uv.y = uvs[3];
		batch->buffer->texid = texSlot;
		batch->buffer++;

		batch->buffer->pos.x = x + tex.width;
		batch->buffer->pos.y = y + tex.height;
		batch->buffer->color.x = r;
		batch->buffer->color.y = g;
		batch->buffer->color.z = b;
		batch->buffer->color.w = 1;
		batch->buffer->uv.x = uvs[4];
		batch->buffer->uv.y = uvs[5];
		batch->buffer->texid = texSlot;
		batch->buffer++;

		batch->buffer->pos.x = x + tex.width;
		batch->buffer->pos.y = y;
		batch->buffer->color.x = r;
		batch->buffer->color.y = g;
		batch->buffer->color.z = b;
		batch->buffer->color.w = 1;
		batch->buffer->uv.x = uvs[6];
		batch->buffer->uv.y = uvs[7];
		batch->buffer->texid = texSlot;
		batch->buffer++;

		batch->indexcount += 6;
		xPos += (font->characters[str[i]]->advance >> 6);
	}
}

INTERNAL inline
void draw_text(RenderBatch* batch, Font* font, std::string str, i32 xPos, i32 yPos, f32 r, f32 g, f32 b) {
	r /= 255;
	g /= 255;
	b /= 255;

	LOCAL f32 FLIP_VER_UVS[8] = { 0, 1, 0, 0, 1, 0, 1, 1 };
	LOCAL f32 FLIP_HOR_UVS[8] = { 1, 1, 1, 0, 0, 0, 0, 1 };
	LOCAL f32 DEFAULT_UVS[8] = { 0, 0, 0, 1, 1, 1, 1, 0 };
	LOCAL f32 FLIP_BOTH_UVS[8] = { 1, 0, 1, 1, 0, 1, 0, 0 };

	for (u16 i = 0; i < str.size(); ++i) {
		Character* c = font->characters[str[i]];
		int yOffset = (font->characters['T']->bearing.y - c->bearing.y) + 1;
		if (yOffset < 0) yOffset = 0;

		int x = xPos + c->bearing.x;
		int y = yPos + yOffset;

		Texture tex = font->characters[str[i]]->texture;
		int texSlot = submit_tex(batch, tex);
		GLfloat* uvs;
		uvs = DEFAULT_UVS;

		batch->buffer->pos.x = x;
		batch->buffer->pos.y = y;
		batch->buffer->color.x = r;
		batch->buffer->color.y = g;
		batch->buffer->color.z = b;
		batch->buffer->color.w = 1;
		batch->buffer->uv.x = uvs[0];
		batch->buffer->uv.y = uvs[1];
		batch->buffer->texid = texSlot;
		batch->buffer++;

		batch->buffer->pos.x = x;
		batch->buffer->pos.y = y + tex.height;
		batch->buffer->color.x = r;
		batch->buffer->color.y = g;
		batch->buffer->color.z = b;
		batch->buffer->color.w = 1;
		batch->buffer->uv.x = uvs[2];
		batch->buffer->uv.y = uvs[3];
		batch->buffer->texid = texSlot;
		batch->buffer++;

		batch->buffer->pos.x = x + tex.width;
		batch->buffer->pos.y = y + tex.height;
		batch->buffer->color.x = r;
		batch->buffer->color.y = g;
		batch->buffer->color.z = b;
		batch->buffer->color.w = 1;
		batch->buffer->uv.x = uvs[4];
		batch->buffer->uv.y = uvs[5];
		batch->buffer->texid = texSlot;
		batch->buffer++;

		batch->buffer->pos.x = x + tex.width;
		batch->buffer->pos.y = y;
		batch->buffer->color.x = r;
		batch->buffer->color.y = g;
		batch->buffer->color.z = b;
		batch->buffer->color.w = 1;
		batch->buffer->uv.x = uvs[6];
		batch->buffer->uv.y = uvs[7];
		batch->buffer->texid = texSlot;
		batch->buffer++;

		batch->indexcount += 6;
		xPos += (font->characters[str[i]]->advance >> 6);
	}
}

INTERNAL inline
void draw_rectangle(RenderBatch* batch, i32 xPos, i32 yPos, i32 width, i32 height, f32 r, f32 g, f32 b, f32 a) {
	r /= 255;
	g /= 255;
	b /= 255;
	a /= 255;

	f32 x = (f32)xPos;
	f32 y = (f32)yPos;

	batch->buffer->pos = { x, y };
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { 0, 0 };
	batch->buffer->texid = 0;
	batch->buffer++;

	batch->buffer->pos = { x, y + height };
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { 0, 0 };
	batch->buffer->texid = 0;
	batch->buffer++;

	batch->buffer->pos = { x + width, y + height };
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { 0, 0 };
	batch->buffer->texid = 0;
	batch->buffer++;

	batch->buffer->pos = { x + width, y };
	batch->buffer->color = { r, g, b, a };
	batch->buffer->uv = { 0, 0 };
	batch->buffer->texid = 0;
	batch->buffer++;

	batch->indexcount += 6;
}

INTERNAL inline
void draw_rectangle(RenderBatch* batch, i32 x, i32 y, i32 width, i32 height, vec4 color) {
	draw_rectangle(batch, x, y, width, height, color.x, color.y, color.z, color.w);
}

//void draw_text(Font& font, const char* str, i32 xPos, i32 yPos, f32 r = 255.0f, f32 g = 255.0f, f32 b = 255.0f);

//void draw_text(Font& font, std::string str, i32 xPos, i32 yPos, f32 r = 255.0f, f32 g = 255.0f, f32 b = 255.0f);

INTERNAL inline
void end2D(RenderBatch* batch) {
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	for (u16 i = 0; i < batch->texcount; ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, batch->textures[i]);
		upload_int(batch->shader, batch->locations[i], i);
	}

	glBindVertexArray(batch->vao);
	glEnableVertexAttribArray(0); //position
	glEnableVertexAttribArray(1); //color
	glEnableVertexAttribArray(2); //texture coordinates
	glEnableVertexAttribArray(3); //texture ID

	glDrawElements(GL_TRIANGLES, batch->indexcount, GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(0); //position
	glDisableVertexAttribArray(1); //color
	glDisableVertexAttribArray(2); //texture coordinates
	glDisableVertexAttribArray(3); //textureID
	glBindVertexArray(0);

	for (u16 i = 0; i < batch->texcount; ++i)
		unbind_texture(batch->textures[i]);

	batch->indexcount = 0;
	batch->texcount = 0;

	stop_shader();
}

INTERNAL inline
Shader load_default_shader_2D() {
	LOCAL const GLchar* ORTHO_SHADER_FRAG_SHADER = R"FOO(
#version 130
out vec4 outColor;

in vec4 pass_color;
in vec2 pass_uv;
in float pass_texid;

uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform sampler2D tex4;
uniform sampler2D tex5;
uniform sampler2D tex6;
uniform sampler2D tex7;
uniform sampler2D tex8;
uniform sampler2D tex9;
uniform sampler2D tex10;
uniform sampler2D tex11;
uniform sampler2D tex12;
uniform sampler2D tex13;
uniform sampler2D tex14;
uniform sampler2D tex15;
uniform sampler2D tex16;
void main() {
	vec4 texColor = vec4(1.0);
	if(pass_texid > 0.0){
		if(pass_texid == 1.0) texColor = texture(tex1, pass_uv);
		if(pass_texid == 2.0) texColor = texture(tex2, pass_uv);
		if(pass_texid == 3.0) texColor = texture(tex3, pass_uv);
		if(pass_texid == 4.0) texColor = texture(tex4, pass_uv);
		if(pass_texid == 5.0) texColor = texture(tex5, pass_uv);
		if(pass_texid == 6.0) texColor = texture(tex6, pass_uv);
		if(pass_texid == 7.0) texColor = texture(tex7, pass_uv);
		if(pass_texid == 8.0) texColor = texture(tex8, pass_uv);
		if(pass_texid == 9.0) texColor = texture(tex9, pass_uv);
		if(pass_texid == 10.0) texColor = texture(tex10, pass_uv);
		if(pass_texid == 11.0) texColor = texture(tex11, pass_uv);
		if(pass_texid == 12.0) texColor = texture(tex12, pass_uv);
		if(pass_texid == 13.0) texColor = texture(tex13, pass_uv);
		if(pass_texid == 14.0) texColor = texture(tex14, pass_uv);
		if(pass_texid == 15.0) texColor = texture(tex15, pass_uv);
		if(pass_texid == 16.0) texColor = texture(tex16, pass_uv);
	}
	outColor = pass_color * texColor;
}

)FOO";

	LOCAL const GLchar* ORTHO_SHADER_VERT_SHADER = R"FOO(
#version 130
in vec2 position;
in vec4 color;
in vec2 uv;
in float texid;

uniform mat4 projection = mat4(1.0);
uniform mat4 view = mat4(1.0);

out vec4 pass_color;
out vec2 pass_uv;
out float pass_texid;

void main() {
	pass_color = color;
	pass_uv = uv;
	pass_texid = texid;
	
	gl_Position = projection * view * vec4(position, 1.0, 1.0);
}

)FOO";
	return load_shader_2D_from_strings(ORTHO_SHADER_VERT_SHADER, ORTHO_SHADER_FRAG_SHADER);
}

INTERNAL inline
void dispose_batch(RenderBatch* batch) {
	glDeleteVertexArrays(1, &batch->vao);
	glDeleteBuffers(1, &batch->vbo);
	glDeleteBuffers(1, &batch->ebo);
	dispose_shader(batch->shader);
}

u32 inline rgba_to_u32(i32 r, i32 g, i32 b, i32 a) {
	return a << 24 | b << 16 | g << 8 | r;
}

#endif