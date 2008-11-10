#include <stdio.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


Board *board;
SDL_Surface *screen;


void
r_setpixel(Uint16 x, Uint16 y, Uint8 r, Uint8 g, Uint8 b)
{
	Uint32 *v32;
	Uint32 c32;

	switch (screen->format->BitsPerPixel) {
		case 8:
			printf("8bpp\n");
			break;
		case 16:
			printf("16bpp\n");
			break;
		case 24:
			printf("24bpp\n");
			break;
		case 32:
			c32 = SDL_MapRGBA(screen->format, r, g, b, 100);
//			c32 = SDL_MapRGB(screen->format, r, g, b);
//			printf("32bpp @ %dx%d=%d -> %d\n", x, y, y * screen->w * 4 + x, c32);
//			v32 = (Uint32*)screen->pixels + y * screen->w + x;
			v32 = screen->pixels + (y * screen->w + x) * 4;
			*v32 = c32;
			break;
		default:
			printf("oops... unknown pixel format.\n");
			exit(-1);
	}
}


void
r_setline(Uint16 x, Uint16 y, Uint16 w, Uint8 r, Uint8 g, Uint8 b)
{
	Uint16 i;

	/* Don't bother for single drops. */
	if (w == 1) {
		r_setpixel(x, y, r, g, b);
		return;
	}

	x = x - w / 2;

	for (i = 0; i < w; i++) {
		r_setpixel(x + i, y, r, g, b);
	}
}


Uint8
handle_events_mouse(SDL_Event *event)
{
	switch ((int)event->button.button) {
		case 5: // mousewheel down
		default:
			board_launch_new_drop(board, 50, 50);
			break;
	}

	return 1;
}


Uint8
handle_events_keyup(SDL_Event *event)
{
	switch ((int)event->key.keysym.sym) {
		case SDLK_DOWN:
			board_set_block_speed(board, SPEED_NORMAL);
			break;
		case SDLK_RIGHT:
			board->moving_right = 0;
			break;
		case SDLK_LEFT:
			board->moving_left = 0;
			break;
		default:
			break;
	}

	return 1;
}


Uint8
handle_events_keydown(SDL_Event *event)
{
	switch ((int)event->key.keysym.sym) {
		case SDLK_ESCAPE:
			return 0;
		case SDLK_F12:
			board_launch_next_block(board);
			break;
		case SDLK_F11:
			board_launch_new_drop(board, 120, 20);
			break;
		case SDLK_F10:
			board_water_all_cubes(board);
			break;
		case SDLK_F9:
			board_dump_drop_map_bmp(board);
			break;
		case SDLK_F8:
			board_random_output(board);
			break;
		case SDLK_F7:
//			board_add_cube(board);
			board_dump_cube_map(board);
			break;
		case SDLK_LEFT:
			board_move_current_block_left(board);
			board_refresh(board);
			board->lateral_tick = 0;
			board->moving_left = 4;
			break;
		case SDLK_RIGHT:
			board_move_current_block_right(board);
			board_refresh(board);
			board->lateral_tick = 0;
			board->moving_right = 4;
			break;
		case SDLK_DOWN:
			board_set_block_speed(board, SPEED_FAST);
			break;
		case SDLK_a:
			board_rotate_cw(board);
			break;
	}
	return 1;
}


/* handle_events() - returning 1 will keep the game playing, returning 0
 * will quit. */
Uint8
handle_events(SDL_Event *event)
{
	switch (event->type) {
		case SDL_KEYDOWN:
			return handle_events_keydown(event);
			break;
		case SDL_KEYUP:
			return handle_events_keyup(event);
			break;
		case SDL_MOUSEBUTTONDOWN:
			return handle_events_mouse(event);
			break;
		default:
			break;
	}

	return 1;
}


void
wait_for_keymouse(void)
{
	SDL_Event event;

	while (1) {
		SDL_WaitEvent(&event);
		switch (event.type) {
			case SDL_MOUSEBUTTONDOWN:
			case SDL_KEYDOWN:
				return;
				break;
			case SDL_QUIT:
				exit(0);
		}
	}
}


void
black_screen(Uint8 s)
{
	memset(screen->pixels, 0, screen->w * screen->h *
			screen->format->BytesPerPixel);
	SDL_Flip(screen);
	SDL_Delay(s * 1000);
}


void
surface_fadein(SDL_Surface *surf)
{
	Uint8 i;

	for (i = 0; i < 127; i++) {
		memset(screen->pixels, 0, screen->w * screen->h *
				screen->format->BytesPerPixel);
		SDL_SetAlpha(surf, SDL_RLEACCEL | SDL_SRCALPHA, i * 2);
		SDL_BlitSurface(surf, NULL, screen, NULL);
		SDL_Flip(screen);
		SDL_Delay(10);
	}
}


void
surface_fadeout(SDL_Surface *surf)
{
	Uint8 i;

	for (i = 0; i < 64; i++) {
		memset(screen->pixels, 0, screen->w * screen->h *
				screen->format->BytesPerPixel);
		SDL_SetAlpha(surf, SDL_RLEACCEL | SDL_SRCALPHA, 255 - i * 4);
		SDL_BlitSurface(surf, NULL, screen, NULL);
		SDL_Flip(screen);
		SDL_Delay(10);
	}
}


void
intro_studio(void)
{
	SDL_Surface *intro;

	intro = loadimage("gfx/zoolu.png");

	surface_fadein(intro);
	SDL_Delay(500);
//	wait_for_keymouse();
	surface_fadeout(intro);

	SDL_FreeSurface(intro);
}


void
intro_title(void)
{
	SDL_Surface *intro;

	intro = loadimage("gfx/rezerwar.png");

	surface_fadein(intro);
	SDL_Delay(500);
//	wait_for_keymouse();
	surface_fadeout(intro);

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

	/* Prepare board */
	board = board_new(10, 20);
	board_loadbg(board, "gfx/gameback.png");

	/* Create main window */
//	screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE|SDL_DOUBLEBUF);
	screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN);
	SDL_WM_SetCaption("rezerwar", NULL);

	/* Slap the original bg. */
	SDL_BlitSurface(board->bg, NULL, screen, NULL);

	/* Load the block textures. */
	block_init_btex();
	cube_init_texture();

	/* Get the first block ready. */
	board_load_next_block(board);
	SDL_Flip(screen);

	/* Zoolu presentation screen. */
//	black_screen(1);
//	intro_studio();
//	intro_title();

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


