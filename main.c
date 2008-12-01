#include <stdio.h>
#include <time.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


Board *board;
SDL_Surface *screen;
SDL_Surface *sprites;


void
intro_studio(void)
{
	SDL_Surface *intro;
	int x;

	intro = loadimage("gfx/zoolu.png");

	x = surface_fadein(intro, 2);
	if (x == 0) x = cancellable_delay(1);
	if (x == 0) surface_fadeout(intro);

	SDL_FreeSurface(intro);
}


void
intro_title(void)
{
	SDL_Surface *intro;
	int x;

	intro = loadimage("gfx/rezerwar.png");

	x = surface_fadein(intro, 2);
	if (x == 0) x = cancellable_delay(1);
	if (x == 0) surface_fadeout(intro);

	SDL_FreeSurface(intro);
}


void
game_menu(void)
{
	SDL_Surface *intro;
	int x;

	intro = loadimage("gfx/gamemenu.png");

	x = surface_fadein(intro, 8);
	osd_print("start new game", 220, 280);
	osd_print("options", 265, 310);
	osd_print("quit", 290, 340);
	SDL_Flip(screen);

	wait_for_keymouse();

	SDL_FreeSurface(intro);
}


void
game_loop()
{
	Uint32 start, now, framecount = 0, fps_lastframe = 0;
	Uint8 playing = 1;
	Uint32 fps_lastframedisplay = 0;
	Sint32 elapsed;
	SDL_Event event;

	/* Main loop, every loop is separated by a TICK (~10ms). 
	 * The board is refreshed every 1/MAXFPS seconds. */
	start = SDL_GetTicks();
	while (playing) {
		while (SDL_PollEvent(&event)) {
			playing = handle_events(&event);
		}

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
}


int
main(int ac, char **av)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		exit(-1);
	}
	atexit(SDL_Quit);

	/* Create main window and seed the random generator. */
	srand(time(NULL));
	screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE|SDL_DOUBLEBUF);
	SDL_WM_SetCaption("rezerwar", NULL);

	/* Prepare board and load the first block. */
	board = board_new(10, 20);
	board_load_next_block(board);

	/* Original graphic load, background, sprites and first Flip. */
	board_loadbg(board, "gfx/gameback.png");
	SDL_BlitSurface(board->bg, NULL, screen, NULL);
	sprites = loadimage("gfx/sprites.png");
	SDL_Flip(screen);

	/* Normal flow... */
	intro_studio();
	intro_title();
	game_menu();
	game_loop();

	/* Death */
	board_kill(board);
	r_checkmem();

	return 0;
}


