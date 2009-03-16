#include <stdio.h>
#include <time.h>

#include "SDL.h"
#include "SDL_mixer.h"

#include "rezerwar.h"


#define BOT_VER "rezerwar alpha 2009-03-08"


Board *board;
Configuration *conf;
SDL_Surface *screen;
SDL_Surface *sprites;
Uint32 key;


void
intro_studio(void)
{
	SDL_Surface *intro;
	int x;

	intro = SDL_LoadBMP("gfx/newgaul/newgaul.bmp");

	x = surface_fadein(intro, 2);
	if (x == 0) x = cancellable_delay(1);
	if (x == 0) surface_fadeout(intro);

	SDL_FreeSurface(intro);
}


void
conf_init()
{
	conf = r_malloc(sizeof(Configuration));
	conf->difficulty = DIFF_EASIEST;
	conf->next_level = NULL;
}


/**
 * This is the game loop, it is called whenever a game has to be started,
 * it will instanciate a board and return when the game is over.
 */
int
game_loop(char *levelname)
{
	Level *level;
	uint32_t start, now, framecount = 0, fps_lastframe = 0,
		  fps_lastframedisplay = 0;
	int elapsed;
	char fpsbuf[16];
	enum mtype status = 0;
	SDL_Event event;

	sfx_play_music("level1");

	/* Prepare board and load the first block. */
	if (levelname == NULL) {
		board = board_new(conf->difficulty);
		board_prepopulate(board, 4);
	} else {
		level = lvl_load(levelname);
		board = board_new_from_level(level);
		lvl_dump(level);
		lvl_kill(level);
	}
	board_add_text(board, (char *)BOT_VER, 10, 450);
	board_load_next_block(board);
	board_launch_next_block(board);

	/* Main loop, every loop is separated by a TICK (~10ms). 
	 * The board is refreshed every 1/MAXFPS seconds. */
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
			board_refresh(board);
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
 * Return whether the user requested sound to run or not.
 */
bool
need_audio(int ac, char **av)
{
	return !has_flag(ac, av, "-nosound");
}


/**
 * Return the fullscreen bit if needed.
 */
uint32_t
need_fullscreen(int ac, char **av)
{
	return has_flag(ac, av, "-fullscreen") == true ? SDL_FULLSCREEN : 0;
}



int
main(int ac, char **av)
{
	int status = MTYPE_SUBMENU;
	uint32_t sdl_flags = 0;
	bool loop = true;

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0)
		fatal("Unable to initialize SDL: %s\n", SDL_GetError());

	/* Set the graphic flags */
	sdl_flags  = SDL_SWSURFACE|SDL_DOUBLEBUF;
	sdl_flags |= need_fullscreen(ac, av);

	atexit(SDL_Quit);

	SDL_EnableUNICODE(1);

	/* Create main window, seed rand, load the sprites and set the alpha. */
	srand(time(NULL));
	screen = SDL_SetVideoMode(640, 480, 32, sdl_flags);
	key = SDL_MapRGB(screen->format, 0, 255, 255);
	SDL_WM_SetCaption("rezerwar", NULL);
	sprites = SDL_LoadBMP("gfx/sprites.bmp");
	SDL_SetColorKey(sprites, SDL_SRCCOLORKEY|SDL_RLEACCEL, key);

	if (need_audio(ac, av))
		init_audio();

	sfx_load_library();
	sfx_play_horn();

	/* Normal flow... */
	intro_studio();
	conf_init();

	/* Loop between game and menu as long as no "quit" was selected. */
	do {
		switch (status) {
			case MTYPE_BREAK:
			case MTYPE_SUBMENU:
				status = main_menu();
				break;
			case MTYPE_NEXTLEVEL:
				status = game_loop(conf->next_level);
				break;
			case MTYPE_QUIT:
				loop = false;
				break;
			case MTYPE_PLAIN:
				status = game_loop(NULL);
				break;
			case MTYPE_START:
				status = game_loop("tuto_01");
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


