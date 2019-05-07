///////////////////////////////////////////////////////////////////////////
// FILE:                       audio.cpp                                 //
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

#include <string.h>
#include "audio.h"

GLOBAL u8 masterVolume;
GLOBAL ALCcontext* context;

struct SoundData {
	u32 sampleCount;
	u32 sampleRate;
	u32 sampleSize;
	u16 channels;
	void* data;
};

INTERNAL inline
SoundData loadWAV(const char* filename) {
	SoundData data = { 0 };
	struct WAVE_HEADER {
		u8  RIFF[4];
		u32 chunkSize;
		u8  WAVE[4];
		u8  fmt[4];
		u32 subChunk1Size;
		u16 audioFormat;
		u16 numOfChannels;
		u32 samplesPerSecond;
		u32 bytesPerSecond;
		u16 blockAlign;
		u16 bitsPerSample;
		u8  subChunk2ID[4];
		u32 subChunk2Size;
	};
	WAVE_HEADER header;

	FILE* file;
	file = fopen(filename, "rb");
	if (file == NULL) {
		BMT_LOG(WARNING, "[%s] Could not open .wav file.", filename);
		return data;
	}
	u32 read = 0;
	read = fread(&header, 1, sizeof(WAVE_HEADER), file);
	if (read == 0) {
		BMT_LOG(WARNING, "[%s] Could not read the .wav file!", filename);
		return data;
	}
	data.channels = header.numOfChannels;
	data.sampleRate = header.samplesPerSecond;
	data.sampleSize = header.bitsPerSample;
	data.sampleCount = (header.subChunk2Size / (data.sampleSize / 8) / data.channels);
	data.data = malloc(header.subChunk2Size);
	fread(data.data, header.subChunk2Size, 1, file);

	fclose(file);

	return data;
}

INTERNAL inline
SoundData loadOGG(const char* filename) {
	SoundData data = { 0 };
	return data;
}

INTERNAL inline
SoundData loadFLAC(const char* filename) {
	SoundData data = { 0 };
	return data;
}

INTERNAL inline
SoundData loadMP3(const char* filename) {
	SoundData data = { 0 };
	return data;
}

void init_audio() {
	ALCdevice *device = alcOpenDevice(NULL);
	if (!device)
		BMT_LOG(FATAL_ERROR, "Could not open audio device!");

	context = NULL;
	context = alcCreateContext(device, NULL);
	if (context == NULL || alcMakeContextCurrent(context) == AL_FALSE) {
		if (context != NULL) alcDestroyContext(context);

		alcCloseDevice(device);

		BMT_LOG(FATAL_ERROR, "Could not initialize audio context!");
	}

	else {
		BMT_LOG(INFO, "Audio device initialized.");

		alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
		alListenerf(AL_GAIN, 1.0f);
		alListener3f(AL_ORIENTATION, 0.0f, 0.0f, -1.0f);
		alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
	}
}

void dispose_audio() {
	ALCdevice *device = alcGetContextsDevice(context);

	if (context == NULL)
		BMT_LOG(WARNING, "Could not get current audio context for closing.");

	alcDestroyContext(context);
	alcCloseDevice(device);
	alcMakeContextCurrent(NULL);

	BMT_LOG(INFO, "Audio device disposed.");
}

void set_master_volume(u8 volume) {
	masterVolume = volume;
	alListenerf(AL_GAIN, (float)volume / 255.0f);
}

u8 get_master_volume() {
	return masterVolume;
}

Sound load_sound(const char* filename) {
	Sound sound = { 0 };
	SoundData data = { 0 };

	if (has_extension(filename, "wav"))       data = loadWAV(filename);
	else if (has_extension(filename, "ogg"))  data = loadOGG(filename);
	else if (has_extension(filename, "flac")) data = loadFLAC(filename);
	else if (has_extension(filename, "mp3"))  data = loadMP3(filename);
	else {
		BMT_LOG(WARNING, "[%s] Extension not supported!", filename);
	}

	sound.format = 0;
	if (data.channels == 1) {
		if (data.sampleSize == 8)       sound.format = AL_FORMAT_MONO8;
		else if (data.sampleSize == 16) sound.format = AL_FORMAT_MONO16;
		else BMT_LOG(WARNING, "Sample size not supported: %i", data.sampleSize);
	}
	else if (data.channels == 2) {
		if (data.sampleSize == 8)       sound.format = AL_FORMAT_STEREO8;
		else if (data.sampleSize == 16) sound.format = AL_FORMAT_STEREO16;
		else BMT_LOG(WARNING, "Sample size not supported: %i", data.sampleSize);
	}
	else
		BMT_LOG(WARNING, "Only MONO and STEREO channels are supported.");

	alGenSources(1, &sound.src);

	if (sound.src == 0) {
		BMT_LOG(WARNING, "[%s] Source not generated for sound!", filename);
	}
	ALenum err = alGetError();
	if (err != AL_NO_ERROR) {
		BMT_LOG(WARNING, "[%s] alGenSources produced an error", filename);
	}

	alSourcef(sound.src, AL_PITCH, 1.0f);
	alSourcef(sound.src, AL_GAIN, 1.0f);
	alSource3f(sound.src, AL_POSITION, 0.0f, 0.0f, 0.0f);
	alSource3f(sound.src, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	alSourcei(sound.src, AL_LOOPING, AL_FALSE);

	alGenBuffers(1, &sound.buffer);

	if (sound.buffer == 0) {
		BMT_LOG(WARNING, "[%s] Buffer not generated for sound!", filename);
	}

	u32 buffer_size = data.channels * data.sampleCount * data.sampleSize / 8;
	alBufferData(sound.buffer, sound.format, data.data, buffer_size, data.sampleRate);
	alSourcei(sound.src, AL_BUFFER, sound.buffer);

	return sound;
}

void play_sound(Sound sound) {
	alSourcePlay(sound.src);
}

void stop_sound(Sound sound) {
	alSourceStop(sound.src);
}

bool is_sound_playing(Sound sound) {
	ALint state;
	alGetSourcei(sound.src, AL_SOURCE_STATE, &state);
	return state == AL_PLAYING;
}

bool is_sound_paused(Sound sound) {
	ALint state;
	alGetSourcei(sound.src, AL_SOURCE_STATE, &state);
	return state == AL_PAUSED;
}

bool is_sound_stopped(Sound sound) {
	ALint state;
	alGetSourcei(sound.src, AL_SOURCE_STATE, &state);
	return state == AL_STOPPED;
}

void set_sound_volume(Sound sound, u8 volume) {
	alSourcef(sound.src, AL_GAIN, (float)volume / 255.0f);
}

void pause_sound(Sound sound) {
	alSourcePause(sound.src);
}

void resume_sound(Sound sound) {
	ALint state;
	alGetSourcei(sound.src, AL_SOURCE_STATE, &state);
	if (state == AL_PAUSED) play_sound(sound);
}

void set_sound_looping(Sound sound, bool loop) {
	alSourcei(sound.src, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
}

void dispose_sound(Sound& sound) {
	alDeleteSources(1, &sound.src);
	alDeleteBuffers(1, &sound.buffer);
	sound.format = 0;
}