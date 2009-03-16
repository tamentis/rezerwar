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
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 512) >= 0)
		has_sound = true;

	if (has_sound)
		Mix_AllocateChannels(16);
}


Mix_Chunk*
sfx_load_sample(char *filename)
{
	Mix_Chunk *sample;

	sample = Mix_LoadWAV(filename);
	if (!sample) {
		fprintf(stderr, "Mix_LoadWAV: %s\n", Mix_GetError());
		exit(-1);
	}

	return sample;
}

void
sfx_unload_library()
{
	if (!has_sound) return;

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
	if (!has_sound) return;

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
sfx_play_music(char *title)
{
	char filename[64];

	if (!has_sound)
		return;

	snprintf(filename, 64, "music/%s.ogg", title);

	Mix_FreeMusic(music);

	// load the song
	if(!(music=Mix_LoadMUS(filename))) {
		fprintf(stderr, "Mix_LoadMUS error (%s)\n", filename);
		exit(-1);
	}

	// set the post mix processor up
	if (Mix_PlayMusic(music, 0)==-1) {
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
