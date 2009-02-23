#include <stdio.h>
#include <time.h>

#include "SDL.h"

#include "rezerwar.h"


#define BOT_VER "rezerwar alpha 2009-02-22"


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
}


void
game_loop()
{
	Uint32 start, now, framecount = 0, fps_lastframe = 0;
	Uint8 playing = 1;
	Uint32 fps_lastframedisplay = 0;
	Sint32 elapsed;
	SDL_Event event;
	Text *t;

	/* Prepare board and load the first block. */
	board = board_new(9, 9, conf->difficulty);
	t = board_add_text(board, (unsigned char *)BOT_VER, 10, 450);
	board_load_next_block(board);
	board_prepopulate(board, 2);
	board_launch_next_block(board);

	/* Main loop, every loop is separated by a TICK (~10ms). 
	 * The board is refreshed every 1/MAXFPS seconds. */
	start = SDL_GetTicks();
	while (playing) {
		while (SDL_PollEvent(&event)) {
			playing = handle_events(&event);
		}

		/* Exit the loop prematurely if we need to leave */
		if (playing != 1)
			break;

		now = SDL_GetTicks();
		board_update(board, now);

		/* Print Frame Per Second. */
		if (now - fps_lastframedisplay > 1000) {
			fprintf(stderr, "FPS: %u\n", framecount);
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
}


int
main(int ac, char **av)
{
	int status = 0;

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		exit(-1);
	}
	atexit(SDL_Quit);

	/* Create main window, seed random, load the sprites and set the alpha. */
	srand(time(NULL));
	screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE|SDL_DOUBLEBUF);
	key = SDL_MapRGB(screen->format, 0, 255, 255);
	SDL_WM_SetCaption("rezerwar", NULL);
	sprites = SDL_LoadBMP("gfx/sprites.bmp");
	SDL_SetColorKey(sprites, SDL_SRCCOLORKEY|SDL_RLEACCEL, key);

	/* Normal flow... */
	intro_studio();
	conf_init();

	/* Loop between game and menu as long as no "quit" was selected. */
	do {
		status = main_menu();
		if (status == 1)
			break;
		game_loop();
	} while (1);

	/* Death */
	r_free(conf);
	r_checkmem();

	return 0;
}


