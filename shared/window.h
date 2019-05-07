///////////////////////////////////////////////////////////////////////////
// FILE:                        window.h                                 //
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

#ifndef WINDOW_H
#define WINDOW_H

#include "maths.h"
#include "defines.h"
#if defined(_WIN32)
#include <windows.h>
#endif
#if defined(__LINUX__)
#include <unistd.h>
#endif
#if defined(__APPLE__)
#include <unistd.h>
#endif

#include <stdlib.h>
#include <vector>

#define MAX_KEYS	1024
#define MAX_BUTTONS	32

void init_window(int width, int height, const char* title, bool fullscreen, bool resizable, bool primary_monitor);

void set_window_pos(int x, int y);
void set_window_size(int width, int height);
void begin_drawing();
void end_drawing();
bool window_closed();
bool window_open();

void set_clear_color(float r, float g, float b, float a);
void set_clear_color(vec4 color);
void set_key_callback(void(*keyCallback)(int key, int action));
void set_mouse_callback(void(*mouseCallback)(double mousex, double mousey, int button, int action));
void set_window_resize_callback(void(*resizecallback)(int width, int height));
void set_scroll_callback(void(*scrollCallback)(double xoffset, double yosffset));

int get_key_pressed();
int get_button_pressed();
int get_key_released();
int get_button_released();
double get_scroll_x();
double get_scroll_y();
bool is_key_pressed(unsigned int keycode);
bool is_key_released(unsigned int keycode);
bool is_button_pressed(unsigned int button);
bool is_button_released(unsigned int button);
bool is_key_down(unsigned int keycode);
bool is_button_down(unsigned int button);
bool is_key_up(unsigned int keycode);
bool is_button_up(unsigned int button);

double get_elapsed_time();

void get_mouse_pos(double* mousexPtr, double* mouseyPtr);
vec2 get_mouse_pos();
void get_mouse_pos(double* mousexPtr, double* mouseyPtr, Rect viewport);
vec2 get_mouse_pos(Rect viewport);
void set_window_should_close(bool shouldClose);
void dispose_window();

void set_fps_cap(double FPS);

enum MouseState {
	MOUSE_LOCKED,
	MOUSE_HIDDEN,
	MOUSE_NORMAL
};

void set_mouse_state(MouseState state);
void set_vsync(bool vsync);

void set_viewport(i32 x, i32 y, i32 width, i32 height);
void resize_viewport(i32 width, i32 height);

int get_window_width();
int get_window_height();
int get_virtual_width();
int get_virtual_height();

INTERNAL inline
Rect fit_aspect_ratio(f32 aspect) {
	if (aspect == 0) aspect = 1;
	f32 screen_width = get_window_width();
	f32 screen_height = get_window_height();

	i32 new_width = screen_width;
	i32 new_height = (i32)(screen_width / aspect);
	if (new_height > screen_height) {
		new_height = screen_height;
		new_width = (i32)(screen_height * aspect);
	}
	return rect((screen_width - new_width) / 2, (screen_height - new_height) / 2, new_width, new_height);
}
#endif