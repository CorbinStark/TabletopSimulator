///////////////////////////////////////////////////////////////////////////
// FILE:                       audio.h                                   //
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

#ifndef AUDIO_H
#define AUDIO_H

#include "defines.h"
#include <alc.h>
#include <al.h>

struct Sound {
	ALuint src;
	ALuint buffer;
	ALint format;
};

void init_audio();
void dispose_audio();

void set_master_volume(u8 volume);
u8 get_master_volume();

Sound load_sound(const char* filename);

bool is_sound_playing(Sound sound);
bool is_sound_paused(Sound sound);
bool is_sound_stopped(Sound sound);
void set_sound_volume(Sound sound, u8 volume);
void play_sound(Sound sound);
void stop_sound(Sound sound);
void pause_sound(Sound sound);
void resume_sound(Sound sound);
void set_sound_looping(Sound sound, bool loop);

void dispose_sound(Sound& sound);

#endif