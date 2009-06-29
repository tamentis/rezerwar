/* $Id$
 *
 * Copyright (c) 2008,2009 Bertrand Janin <tamentis@neopulsar.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <stdio.h>
#include <time.h>

#include "SDL.h"
#include "SDL_mixer.h"

#include "rezerwar.h"

bool has_sound = false;

Mix_Music *music = NULL;
Mix_Chunk *tick1;
Mix_Chunk *tack1;
Mix_Chunk *horn;
Mix_Chunk *boom;
Mix_Chunk *lazer;
Mix_Chunk *menunav;
Mix_Chunk *menuselect;

void
init_audio()
{
	/* Open a mixer */
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) >= 0)
		has_sound = true;

	if (has_sound)
		Mix_AllocateChannels(16);
}


Mix_Chunk*
sfx_load_sample(char *filename)
{
	Mix_Chunk *sample;
	const char *path = dpath(filename);

	sample = Mix_LoadWAV(path);

	if (!sample) {
		fprintf(stderr, "Mix_LoadWAV: %s\n", Mix_GetError());
		exit(-1);
	}

	return sample;
}

void
sfx_unload_library()
{
	if (!has_sound)
		return;

	Mix_FreeChunk(tick1);
	Mix_FreeChunk(tack1);
	Mix_FreeChunk(horn);
	Mix_FreeChunk(boom);
	Mix_FreeChunk(lazer);
	Mix_FreeChunk(menunav);
	Mix_FreeChunk(menuselect);
}


void
sfx_load_library()
{
	if (!has_sound)
		return;

	tick1 = sfx_load_sample("sfx/tick1.ogg");
	tack1 = sfx_load_sample("sfx/tack1.ogg");
	horn = sfx_load_sample("sfx/horn.ogg");
	boom = sfx_load_sample("sfx/boom.ogg");
	lazer = sfx_load_sample("sfx/lazer.ogg");
	menunav = sfx_load_sample("sfx/menunav.ogg");
	menuselect = sfx_load_sample("sfx/menuselect.ogg");
}


void sfx_play_tack1() { if (has_sound) Mix_PlayChannel(-1, tack1, 0); }
void sfx_play_tick1() { if (has_sound) Mix_PlayChannel(-1, tick1, 0); }
void sfx_play_horn() { if (has_sound) Mix_PlayChannel(-1, horn, 0); }
void sfx_play_boom() { if (has_sound) Mix_PlayChannel(-1, boom, 0); }
void sfx_play_lazer() { if (has_sound) Mix_PlayChannel(-1, lazer, 0); }
void sfx_play_menunav() { if (has_sound) Mix_PlayChannel(-1, menunav, 0); }
void sfx_play_menuselect() { if (has_sound) Mix_PlayChannel(-1, menuselect, 0); }

void
sfx_play_music(char *filename)
{
	char *path;

	if (!has_sound)
		return;
	
	Mix_FreeMusic(music);

	path = dpath(filename);
	// load the song
	if(!(music=Mix_LoadMUS(path))) {
		fprintf(stderr, "Mix_LoadMUS error (%s)\n", filename);
		exit(-1);
	}
	r_free(path);

	// set the post mix processor up
	if (Mix_PlayMusic(music, -1)==-1) {
		fprintf(stderr, "Mix_LoadMUS error\n");
		exit(-1);
	}
}

void
sfx_stop_music()
{
	Mix_FadeOutMusic(200);
	Mix_FreeMusic(music);
	music = NULL;
}

void
sfx_toggle_mute(bool yup)
{
	if (yup) {
		Mix_Volume(-1, 0);
		Mix_VolumeMusic(0);
	} else {
		Mix_Volume(-1, MIX_MAX_VOLUME);
		Mix_VolumeMusic(MIX_MAX_VOLUME);
	}
}
