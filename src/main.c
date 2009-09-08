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
#ifdef __WII__
#  include <fat.h>
#endif

#include "SDL.h"
#include "SDL_mixer.h"

#include "rezerwar.h"
#include "config.h"


#define VERSION "rezerwar 0.4"


Board *board;
Configuration *conf;
SDL_Surface *screen;
SDL_Surface *sprites;
Uint32 key;


/**
 * Fade the initial studio screen.
 */
void
intro_studio(void)
{
	SDL_Surface *intro;
	char *path;
	int x;

	path = dpath("gfx/tdc.bmp");
	intro = SDL_LoadBMP(path);
	r_free(path);

	if (intro == NULL)
		fatal("Unable to load tdc intro image.");

	x = gfx_fadein(intro, 4);
	if (x == 0)
		x = cancellable_delay(1);

	gfx_free(intro);
}


/**
 * Check for the presence of an cmd line flag.
 */
bool
has_flag(int ac, char **av, char *flag)
{
	int i;

	if (ac <= 1)
		return false;

	for (i = 0; i < ac; i++) {
		if (strcmp(av[i], flag) == 0)
			return true;
	}

	return false;
}


/**
 * Configuration constructor.
 */
void
init_conf(int ac, char **av)
{
	conf = r_malloc(sizeof(Configuration));
	conf->current_level = NULL;
	conf->next_level = NULL;
	conf->sound = true;
	conf->fullscreen = has_flag(ac, av, "-f");
}


/**
 * This is the game loop, it is called whenever a game has to be started,
 * it will instanciate a board and return when the game is over.
 */
int
game_loop(char *levelname, enum ttype trans)
{
	Level *level;
	uint32_t	start,		// when the game loop starts
			now,		// time at the start of each loops
			framecount = 0,	// nr of frame drawn in this TICK
			lastframe = 0,	// whenever the last frame was drawn
		  	lastfps = 0;	// whenever the last fps was drawn
	int		elapsed;	// how long did the loop take
	char		fpsbuf[16];	// temp char* for the fps
	enum mtype	status = 0;	// current mode (menu, game, etc..)
	SDL_Event	event;		// last loop event

	sfx_play_music("music/level1.mp3");

	/*
	 * Without explicit level name, start the board with 4 lines of cubes
	 * and two moles.
	 */
	if (levelname == NULL) {
		board = board_new();
		board_prepopulate(board, 4);

	/* With a level name, load everything from the level. */
	} else {
		level = lvl_load(levelname);
		board = board_new_from_level(level);
		/* lvl_dump(level); */
		lvl_kill(level);
	}
	board->transition = trans;
	board_load_next_cube(board);
	board_launch_next_cube(board);

	/*
	 * Main loop, every TICK (~10ms)
	 *  - poll events and handle them, updating the game internals if needed.
	 *  - go through the normal internal updates (not event-related).
	 *  - draw the screen according to MAXFPS
	 *  - wait whatever is left to get to a 10ms TICK.
	 */
	start = SDL_GetTicks();
	while (status == MTYPE_NOP) {
		while (SDL_PollEvent(&event)) {
			status = handle_events(&event);
		}

		/* If a user event triggered a loop exit, do it now */
		if (status != MTYPE_NOP)
			break;

		now = SDL_GetTicks();
		status = board_update(board, now);

		/* Print FPS... every seconds. */
		if (now - lastfps > 1000) {
			if (board->show_fps) {
				snprintf(fpsbuf, 16, "FPS: %u\n", framecount);
				text_set_value(board->fps_t, fpsbuf);
			} else {
				text_set_value(board->fps_t, "");
			}
			lastfps = now;
			framecount = 0;
		}

		/* Every 1.0 / MAXFPS seconds, refresh the screen. */
		if (lastframe < (now - (1000/MAXFPS))) {
			framecount++;
			lastframe = now;
			board_render(board);
		}

		elapsed = SDL_GetTicks() - now;
		if (elapsed < TICK) {
			SDL_Delay(TICK - elapsed);
		}
	}

	board_kill(board);
	sfx_stop_music();

	return status;
}


/**
 * Return whether the user requested sound to run or not.
 */
bool
need_audio(int ac, char **av)
{
	return !has_flag(ac, av, "-q");
}


/**
 * Run when the user instanciate the executable with -h.
 */
void
help(const char *progname)
{
	printf("usage: %s [-hvqf] levelfile\n", progname);
	printf("    -h     This help screen\n");
	printf("    -v     Just version number\n");
	printf("    -q     Quiet, no sound\n");
	printf("    -f     Fullscreen\n");
}


/**
 * Big bang.
 */
int
main(int ac, char **av)
{
	int status = MTYPE_SUBMENU;
	bool loop = true;
	char *path;
	SDL_Joystick *js;

	/* Version number only */
	if (has_flag(ac, av, "-v")) {
		printf("%s\n", VERSION);
		return 0;
	}

	/* Help request */
	if (has_flag(ac, av, "-h") || has_flag(ac, av, "--help") ||
			has_flag(ac, av, "-help")) {
		help(av[0]);
		return 0;
	}

	/* Load the sprites first, avoid running init if something is fishy */
	path = dpath("gfx/sprites.bmp");
	sprites = SDL_LoadBMP(path);
	if (sprites == NULL)
		fatal("Unable to load the sprites, did you install rezerwar properly?");
	SDL_SetColorKey(sprites, SDL_SRCCOLORKEY|SDL_RLEACCEL, 
			SDL_MapRGB(sprites->format, 0, 255, 255));
	r_free(path);

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK) != 0)
		fatal("Unable to initialize SDL: %s\n", SDL_GetError());

	SDL_JoystickEventState(SDL_ENABLE);
	js = SDL_JoystickOpen(0);

#ifdef __WII__
	fatInitDefault();
#endif
	SDL_EnableUNICODE(1);

	init_conf(ac, av);

	gfx_init();

	/* Initialize the cube randomizer */
	cube_init_rmap();

	atexit(SDL_Quit);

	/* Seed rand, load the sprites and set the alpha. */
	srand(time(NULL));
	key = SDL_MapRGB(screen->format, 0, 255, 255);
	SDL_WM_SetCaption(VERSION, NULL);

	if (need_audio(ac, av))
		sfx_init();

	sfx_load_library();
	sfx_play_horn();

	/* DEBUG / XXX */
	// return game_loop(NULL, 0);
	// return game_loop("tuto_04", 0);
	return game_loop("tuto_08", 0);

	/* Normal flow... */
	intro_studio();

	/* Loop between game and menu as long as no "quit" was selected. */
	do {
		switch (status) {
		case MTYPE_BREAK:
		case MTYPE_SUBMENU:
			status = main_menu();
			break;
		case MTYPE_NEXTLEVEL:
			r_free(conf->current_level);
			conf->current_level = r_strcp(conf->next_level);
			status = game_loop(conf->next_level, TTYPE_NONE);
			break;
		case MTYPE_REPLAY:
			status = game_loop(conf->current_level, TTYPE_NONE);
			break;
		case MTYPE_GAMEOVER_WIN:
			status = gameover_menu(status);
			break;
		case MTYPE_GAMEOVER_LOSE:
			status = gameover_menu(status);
			break;
		case MTYPE_GAMEOVER_TIMEOUT:
			status = gameover_menu(status);
			break;
		case MTYPE_GAMEOVER_HISCORE:
			status = hiscore_prompt();
			break;
		case MTYPE_HISCORES:
			status = MTYPE_SUBMENU;
			hiscore_show();
			break;
		case MTYPE_QUIT:
			loop = false;
			break;
		case MTYPE_PLAIN:
			gfx_shutter_close();
			status = game_loop(NULL, TTYPE_SHUTTER_OPEN);
			break;
		case MTYPE_START:
			gfx_shutter_close();
			r_free(conf->current_level);
			conf->current_level = r_strcp("tuto_01");

			status = game_loop(conf->current_level, TTYPE_SHUTTER_OPEN);
			break;
		}
	} while (loop);

	/* Death and cleanup */
	SDL_FreeSurface(sprites);
	sfx_unload_library();
	Mix_CloseAudio();
	hiscore_free();
	r_free(conf->next_level);
	r_free(conf);
	r_checkmem();

	return 0;
}
