#include <stdio.h>
#include <time.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


Board *board;
SDL_Surface *screen;


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
	wait_for_keymouse();

	SDL_FreeSurface(intro);
}


int
main(int ac, char **av)
{
	Uint32 start, now, framecount = 0, fps_lastframe = 0;
	Uint8 playing = 1;
	Uint32 fps_lastframedisplay = 0;
	Sint32 elapsed;
	SDL_Event event;

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		exit(-1);
	}
	atexit(SDL_Quit);

	srand(time(NULL));

	/* Prepare board */
	board = board_new(10, 20);
	board_loadbg(board, "gfx/gameback.png");

	/* Create main window */
	screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE|SDL_DOUBLEBUF);
//	screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN);
	SDL_WM_SetCaption("rezerwar", NULL);

	/* Slap the original bg. */
	SDL_BlitSurface(board->bg, NULL, screen, NULL);

	/* Load the block textures. */
	block_init_btex();
	cube_init_texture();

	/* Get the first block ready. */
	board_load_next_block(board);
	SDL_Flip(screen);

	/* Presentation screens. */
	intro_studio();
	intro_title();

	/* Game menu */
	game_menu();

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

	board_kill(board);

	r_checkmem();

	return 0;
}


