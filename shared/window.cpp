///////////////////////////////////////////////////////////////////////////
// FILE:                       window.cpp                                //
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

#include <thread>
#include "window.h"

GLOBAL GLFWwindow* glfw_window;
GLOBAL i32 winVirtualWidth;
GLOBAL i32 winVirtualHeight;
GLOBAL i32 winx;
GLOBAL i32 winy;
GLOBAL i32 winwidth;
GLOBAL i32 winheight;
GLOBAL f64 mousex;
GLOBAL f64 mousey;

GLOBAL MouseState mouseState;

GLOBAL f64 targetFPS;
GLOBAL i32 keys[MAX_KEYS];
GLOBAL i32 buttons[MAX_BUTTONS];

GLOBAL f64 currentTime, previousTime;
GLOBAL f64 updateTime, drawTime;
GLOBAL f64 frameTime = 0.0;
GLOBAL f64 targetTime = 0.0;

GLOBAL void(*BMTKeyCallback)(int key, int action);
GLOBAL void(*BMTMouseCallback)(double mousex, double mousey, int button, int action);
GLOBAL void(*BMTResizeCallback)(int width, int height);
GLOBAL void(*BMTScrollCallback)(double xoffset, double yoffset);

GLOBAL int lastKeyPressed;
GLOBAL int lastButtonPressed;
GLOBAL int lastKeyReleased;
GLOBAL int lastButtonReleased;
GLOBAL double lastScrollX;
GLOBAL double lastScrollY;

//TODO: implement the GUI into this engine.
INTERNAL
void keycallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
	keys[key] = action;
	if (BMTKeyCallback != NULL)
		BMTKeyCallback(key, action);
	//Panel::key_callback_func(key, action);

	if (action == GLFW_PRESS)
		lastKeyPressed = key;
	else if (action == GLFW_RELEASE)
		lastKeyReleased = key;
}

INTERNAL
void char_callback(GLFWwindow* win, unsigned int key) {
	lastKeyPressed = key;
}

INTERNAL
void cursorPosCallback(GLFWwindow* win, double xPos, double yPos) {
	mousex = xPos;
	mousey = yPos;
	//Panel::mouse_pos_callback_func(xPos, yPos);
}

INTERNAL
void mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) {
	buttons[button] = action;
	if (BMTMouseCallback != NULL)
		BMTMouseCallback(get_mouse_pos().x, get_mouse_pos().y, button, action);
	//Panel::mouse_callback_func(button, action, mousex, mousey);

	if (action == GLFW_PRESS)
		lastButtonPressed = button;
	else if (action == GLFW_RELEASE)
		lastButtonReleased = button;
}

INTERNAL
void resizeCallback(GLFWwindow* win, int width, int height) {
	if (BMTResizeCallback != NULL)
		BMTResizeCallback(width, height);
	set_window_size(width, height);
}

INTERNAL
void scrollCallback(GLFWwindow* win, double xoffset, double yoffset) {
	if (BMTScrollCallback != NULL)
		BMTScrollCallback(xoffset, yoffset);
	lastScrollX = xoffset;
	lastScrollY = yoffset;
}

void init_window(int window_width, int window_height, const char* title, bool fullscreen, bool resizable, bool primary_monitor) {
	winwidth = window_width;
	winheight = window_height;
	winVirtualWidth = winwidth;
	winVirtualHeight = winheight;

	lastKeyPressed = lastButtonPressed = 0;
	lastScrollX = 0;
	lastScrollY = 0;

	//INIT GLFW
	if (!glfwInit()) {
		BMT_LOG(MINOR_ERROR, "GLFW could not initialize");
	}
	else {
		BMT_LOG(INFO, "GLFW has initialized");
	}

	for (int i = 0; i < MAX_KEYS; ++i)
		keys[i] = -1;
	for (int i = 0; i < MAX_BUTTONS; ++i)
		buttons[i] = -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	//END INIT GLFW

	glfwWindowHint(GLFW_RESIZABLE, resizable);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	if (fullscreen) {
		glfwWindowHint(GLFW_VISIBLE, false);
		glfwWindowHint(GLFW_DECORATED, false);
		glfw_window = glfwCreateWindow(winwidth, winheight, title, NULL, NULL);
		if (!glfw_window) {
			BMT_LOG(MINOR_ERROR, "Windowed Window failed to be created");
			glfwTerminate();
		}
		winx = winy = 0;

		set_window_size(
			glfwGetVideoMode(glfwGetPrimaryMonitor())->width,
			glfwGetVideoMode(glfwGetPrimaryMonitor())->height
		);

		glfwSetWindowPos(glfw_window, 0, 0);
		if (!primary_monitor)
			glfwSetWindowPos(glfw_window, glfwGetVideoMode(glfwGetPrimaryMonitor())->width, 0);
	}
	else {
		glfwWindowHint(GLFW_VISIBLE, false);
		glfw_window = glfwCreateWindow(winwidth, winheight, title, NULL, NULL);
		if (!glfw_window) {
			BMT_LOG(MINOR_ERROR, "Windowed Window failed to be created");
			glfwTerminate();
		}
		winx = winy = 0;

		float scrWidth = (float)glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
		float scrHeight = (float)glfwGetVideoMode(glfwGetPrimaryMonitor())->height;

		glfwSetWindowPos(glfw_window, (int)((scrWidth / 2) - (winwidth / 2)), (int)((scrHeight / 2) - (winheight / 2)));
		if (!primary_monitor)
			glfwSetWindowPos(glfw_window, (int)(glfwGetVideoMode(glfwGetPrimaryMonitor())->width + (scrWidth / 2) - (winwidth / 2)),
			(int)((scrHeight / 2) - (winheight / 2))
			);
	}

	//INIT GLEW
	glfwMakeContextCurrent(glfw_window);
	glfwSwapInterval(0);
	glfwShowWindow(glfw_window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		BMT_LOG(FATAL_ERROR, "Failed to initialize GLAD\n");
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	BMT_LOG(INFO, "OpenGL Version: %s", glGetString(GL_VERSION));
	BMT_LOG(INFO, "GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	BMT_LOG(INFO, "OpenGL Vendor: %s", glGetString(GL_VENDOR));
	BMT_LOG(INFO, "Graphics Card: %s", glGetString(GL_RENDERER));

	glfwSetKeyCallback(glfw_window, keycallback);
	glfwSetWindowSizeCallback(glfw_window, resizeCallback);
	glfwSetMouseButtonCallback(glfw_window, mouseButtonCallback);
	glfwSetCursorPosCallback(glfw_window, cursorPosCallback);
	glfwSetCharCallback(glfw_window, char_callback);
	glfwSetScrollCallback(glfw_window, scrollCallback);
}

void set_window_pos(int window_x, int window_y) {
	winx = window_x;
	winy = window_y;
	glfwSetWindowPos(glfw_window, winx, winy);
}

void set_window_size(int window_width, int window_height) {
	if (window_width < 1) window_width = 1;
	if (window_height < 1) window_height = 1;
	winwidth = window_width;
	winheight = window_height;
	glfwSetWindowSize(glfw_window, winwidth, winheight);
}

void set_clear_color(float r, float g, float b, float a) {
	glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

void set_clear_color(vec4 color) {
	set_clear_color(color.x, color.y, color.z, color.w);
}

void begin_drawing() {
	currentTime = glfwGetTime();
	updateTime = currentTime - previousTime;
	previousTime = currentTime;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void end_drawing() {
	for (int i = 0; i < MAX_KEYS; ++i) {
		keys[i] = -1;
	}
	for (int i = 0; i < MAX_BUTTONS; ++i) {
		buttons[i] = -1;
	}
	lastKeyPressed = 0;
	lastButtonPressed = 0;
	lastKeyReleased = 0;
	lastButtonReleased = 0;
	lastScrollX = 0;
	lastScrollY = 0;

	glfwSwapBuffers(glfw_window);
	glfwPollEvents();

	currentTime = glfwGetTime();
	drawTime = currentTime - previousTime;
	previousTime = currentTime;

	frameTime = updateTime + drawTime;

	// Wait for some milliseconds...
	if (frameTime < targetTime)
	{
		double prevTime = glfwGetTime();
		double nextTime = 0.0;

#if defined(_BUSY_WAIT)
		// Busy wait loop
		while ((nextTime - prevTime) < ((targetTime - frameTime)*1000.0f) / 1000.0f) nextTime = glfwGetTime();
#elif defined(_WIN32) || defined(_WIN64)
		Sleep((DWORD)((targetTime - frameTime)*1000.0f));
#elif defined(__LINUX__)
		usleep((targetTime - frameTime)*1000.0f);
#elif defined(__APPLE__)
		usleep((targetTime - frameTime)*1000.0f);
#endif

		currentTime = glfwGetTime();
		double extraTime = currentTime - previousTime;
		previousTime = currentTime;

		frameTime += extraTime;
	}
#if 0
	static int framecount = 0;
	framecount++;
	if (framecount > 150) {
		std::cout << "FPS:" << (int)(1.0f / (float)frameTime) << std::endl;
		framecount = 0;
	}
#endif

}

double get_elapsed_time() {
	return glfwGetTime();
}

void set_fps_cap(double FPS) {
	if (FPS < 1) targetTime = 0.0;
	else targetTime = 1.0 / FPS;
}

bool window_closed() {
	return glfwWindowShouldClose(glfw_window) == 1;
}

bool window_open() {
	return glfwWindowShouldClose(glfw_window) == 0;
}

void set_key_callback(void(*keyCallback)(int key, int action)) {
	BMTKeyCallback = keyCallback;
}

void set_mouse_callback(void(*mouseCallback)(double mousex, double mousey, int button, int action)) {
	BMTMouseCallback = mouseCallback;
}

void set_window_resize_callback(void(*resizecallback)(int width, int height)) {
	BMTResizeCallback = resizecallback;
}

void set_scroll_callback(void(*scrollCallback)(double xoffset, double yoffset)) {
	BMTScrollCallback = scrollCallback;
}

int get_key_pressed() {
	return lastKeyPressed;
}

int get_button_pressed() {
	return lastButtonPressed;
}

int get_key_released() {
	return lastKeyReleased;
}

int get_button_released() {
	return lastButtonReleased;
}

double get_scroll_x() {
	return lastScrollX;
}

double get_scroll_y() {
	return lastScrollY;
}

bool is_key_pressed(unsigned int keycode) {
	if (keys[keycode] == GLFW_PRESS)
		return true;
	return false;
}

bool is_key_released(unsigned int keycode) {
	if (keys[keycode] == GLFW_RELEASE) {
		return true;
	}
	return false;
}

bool is_button_pressed(unsigned int button) {
	if (buttons[button] == GLFW_PRESS) {
		return true;
	}
	return false;
}

bool is_button_released(unsigned int button) {
	if (buttons[button] == GLFW_RELEASE) {
		return true;
	}
	return false;
}

bool is_key_down(unsigned int keycode) {
	if (glfwGetKey(glfw_window, keycode) == 1) {
		return true;
	}
	return false;
}

bool is_button_down(unsigned int button) {
	if (glfwGetMouseButton(glfw_window, button) == 1) {
		return true;
	}
	return false;
}

bool is_key_up(unsigned int keycode) {
	if (glfwGetKey(glfw_window, keycode) == 0) {
		return true;
	}
	return false;
}

bool is_button_up(unsigned int button) {
	if (glfwGetMouseButton(glfw_window, button) == 0) {
		return true;
	}
	return false;
}

void set_vsync(bool vsync) {
	if (vsync)
		glfwSwapInterval(1);
	else
		glfwSwapInterval(0);
}

void set_viewport(i32 x, i32 y, i32 width, i32 height) {
	glViewport(x, y, width, height);
}

void resize_viewport(i32 width, i32 height) {
	glViewport(0, 0, width, height);
}

void get_mouse_pos(double* mousexPtr, double* mouseyPtr) {
	*mousexPtr = mousex;
	*mouseyPtr = mousey;
}

vec2 get_mouse_pos() {
	return V2((float)mousex, (float)mousey);
}

void get_mouse_pos(double* mousexPtr, double* mouseyPtr, Rect viewport) {
	*mousexPtr = mousex;
	*mouseyPtr = mousey;

	*mousexPtr -= viewport.x;
	*mouseyPtr -= viewport.y;
	//*mousexPtr /= ((f32)viewport.width / (f32)get_virtual_width());
	//*mouseyPtr /= ((f32)viewport.height / (f32)get_virtual_height());
}

vec2 get_mouse_pos(Rect viewport) {
	vec2 mousepos = V2((f32)mousex, (f32)mousey);
	mousepos.x -= viewport.x;
	mousepos.y -= viewport.y;
	//mousepos.x /= ((f32)viewport.width / (f32)get_virtual_width());
	//mousepos.y /= ((f32)viewport.height / (f32)get_virtual_height());
	return mousepos;
}

void set_window_should_close(bool shouldClose) {
	glfwSetWindowShouldClose(glfw_window, shouldClose);
}

void dispose_window() {
	glfwSetWindowShouldClose(glfw_window, true);
	glfwDestroyWindow(glfw_window);
	glfwDefaultWindowHints();
	glfwTerminate();
}

void set_mouse_state(MouseState state) {
	mouseState = state;
	if (state == MOUSE_LOCKED)
		glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else if (state == MOUSE_HIDDEN)
		glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	else
		glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

int get_window_width() {
	int win_width;
	glfwGetWindowSize(glfw_window, &win_width, 0);
	return win_width;
}

int get_window_height() {
	int win_height;
	glfwGetWindowSize(glfw_window, 0, &win_height);
	return win_height;
}

int get_virtual_width() {
	return winVirtualWidth;
}

int get_virtual_height() {
	return winVirtualHeight;
}