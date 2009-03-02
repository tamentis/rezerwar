#include <stdio.h>
#include <time.h>

#include "SDL.h"
#include "SDL_mixer.h"

#include "rezerwar.h"

Mix_Chunk *tick1;
Mix_Chunk *tack1;
Mix_Chunk *horn;

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
sfx_load_library()
{
	tick1 = sfx_load_sample("sfx/tick1.ogg");
	tack1 = sfx_load_sample("sfx/tack1.ogg");
	horn = sfx_load_sample("sfx/horn.ogg");
}


void sfx_play_tack1() { Mix_PlayChannel(-1, tack1, 0); }
void sfx_play_tick1() { Mix_PlayChannel(-1, tick1, 0); }
void sfx_play_horn() { Mix_PlayChannel(-1, horn, 0); }

void
sfx_play_music()
{
	Mix_Music *music;

	// load the song
	if(!(music=Mix_LoadMUS("sfx/music.mp3"))) {
		fprintf(stderr, "Mix_LoadMUS error\n");
		exit(-1);
	}

	// set the post mix processor up
	if (Mix_PlayMusic(music, 1)==-1) {
		fprintf(stderr, "Mix_LoadMUS error\n");
		exit(-1);
	}
}
