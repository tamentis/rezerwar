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
#include <fat.h>
#endif

#include "SDL.h"
#include "SDL_mixer.h"

#include "rezerwar.h"
#include "../config.h"


#define VERSION "rezerwar 0.3"


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

	x = surface_fadein(intro, 4);
	if (x == 0)
		x = cancellable_delay(1);

	SDL_FreeSurface(intro);
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
	conf->difficulty = DIFF_EASIEST;
	conf->current_level = NULL;
	conf->next_level = NULL;
	conf->sound = true;
	conf->fullscreen = has_flag(ac, av, "-fullscreen");
}


/**
 * This is the game loop, it is called whenever a game has to be started,
 * it will instanciate a board and return when the game is over.
 */
int
game_loop(char *levelname, enum ttype trans)
{
	Level *level;
	uint32_t start, now, framecount = 0, fps_lastframe = 0,
		  fps_lastframedisplay = 0;
	int elapsed;
	char fpsbuf[16];
	enum mtype status = 0;
	SDL_Event event;

	sfx_play_music("music/level1.mp3");

	/* Prepare board and load the first block. */
	if (levelname == NULL) {
		board = board_new(conf->difficulty);
		board_prepopulate(board, 4);
		board_spawn_mole(board);
		board_spawn_mole(board);
	} else {
		level = lvl_load(levelname);
		board = board_new_from_level(level);
		/* lvl_dump(level); */
		lvl_kill(level);
	}
	board->transition = trans;
	board_load_next_block(board);
	board_launch_next_block(board);

	/*
	 * Main loop, every loop is separated by a TICK (~10ms). 
	 * The board is refreshed every 1/MAXFPS seconds.
	 */
	start = SDL_GetTicks();
	while (status == MTYPE_NOP) {
		while (SDL_PollEvent(&event)) {
			status = handle_events(&event);
		}

		/* Exit the loop prematurely if we need to leave */
		if (status != MTYPE_NOP)
			break;

		now = SDL_GetTicks();
		status = board_update(board, now);

		/* Print Frame Per Second. */
		if (now - fps_lastframedisplay > 1000) {
			if (board->show_fps) {
				snprintf(fpsbuf, 16, "FPS: %u\n", framecount);
				text_set_value(board->fps_t, fpsbuf);
			} else {
				text_set_value(board->fps_t, "");
			}
			fps_lastframedisplay = now;
			framecount = 0;
		}

		/* Every 1.0 / MAXFPS seconds, refresh the screen. */
		if (fps_lastframe < (now - (1000/MAXFPS))) {
			framecount++;
			fps_lastframe = now;
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
	return !has_flag(ac, av, "-nosound");
}



int
main(int ac, char **av)
{
	int status = MTYPE_SUBMENU;
	bool loop = true;
	char *path;
	SDL_Joystick *js;

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

	init_gfx();

	cube_init_rmap();

	atexit(SDL_Quit);

	/* Seed rand, load the sprites and set the alpha. */
	srand(time(NULL));
//	screen = SDL_SetVideoMode(640, 480, 32, sdl_flags);
	key = SDL_MapRGB(screen->format, 0, 255, 255);
	SDL_WM_SetCaption(VERSION, NULL);
//	SDL_ShowCursor(false);

	if (need_audio(ac, av))
		init_audio();

	sfx_load_library();
	sfx_play_horn();

	/* Normal flow... */
//	intro_studio();

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
				surface_pixel_close();
//				surface_shutter_close();
				status = game_loop(NULL, TTYPE_PIXEL_OPEN);
				break;
			case MTYPE_START:
				surface_shutter_close();
				r_free(conf->current_level);
				conf->current_level = r_strcp("tuto_01");

				status = game_loop(conf->current_level, TTYPE_SHUTTER_OPEN);
				break;
		}
	} while (loop);

	/* Death */
	sfx_unload_library();
	Mix_CloseAudio();
	hiscore_free();
	r_free(conf->next_level);
	r_free(conf);
	r_checkmem();

	return 0;
}


