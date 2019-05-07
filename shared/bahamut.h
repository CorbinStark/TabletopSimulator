///////////////////////////////////////////////////////////////////////////
// FILE:                      bahamut.h                                  //
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

#ifndef BAHAMUT_H
#define BAHAMUT_H

#include "audio.h"
#include "defines.h"
#include "maths.h"
#include "render2D.h"
#include "font.h"
#include "shader.h"
#include "texture.h"
#include "window.h"

INTERNAL vec4 LIGHTGRAY = V4(200, 200, 200, 255);
INTERNAL vec4 GRAY = V4(130, 130, 130, 255);
INTERNAL vec4 DARKGRAY = V4(80, 80, 80, 255);
INTERNAL vec4 YELLOW = V4(253, 249, 0, 255);
INTERNAL vec4 GOLD = V4(255, 203, 0, 255);
INTERNAL vec4 ORANGE = V4(255, 161, 0, 255);
INTERNAL vec4 PINK = V4(255, 109, 194, 255);
INTERNAL vec4 RED = V4(230, 41, 55, 255);
INTERNAL vec4 MAROON = V4(190, 33, 55, 255);
INTERNAL vec4 GREEN = V4(0, 228, 48, 255);
INTERNAL vec4 LIME = V4(0, 158, 47, 255);
INTERNAL vec4 DARKGREEN = V4(0, 117, 44, 255);
INTERNAL vec4 SKYBLUE = V4(102, 191, 255, 255);
INTERNAL vec4 BLUE = V4(0, 121, 241, 255);
INTERNAL vec4 DARKBLUE = V4(0, 82, 172, 255);
INTERNAL vec4 PURPLE = V4(200, 122, 255, 255);
INTERNAL vec4 VIOLET = V4(135, 60, 190, 255);
INTERNAL vec4 DARKPURPLE = V4(112, 31, 126, 255);
INTERNAL vec4 BEIGE = V4(211, 176, 131, 255);
INTERNAL vec4 BROWN = V4(127, 106, 79, 255);
INTERNAL vec4 DARKBROWN = V4(76, 63, 47, 255);

INTERNAL vec4 WHITE = V4(255, 255, 255, 255);
INTERNAL vec4 BLACK = V4(0, 0, 0, 255);
INTERNAL vec4 BLANK = V4(0, 0, 0, 0);
INTERNAL vec4 MAGENTA = V4(255, 0, 255, 255);

#endif