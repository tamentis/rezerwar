#include <stdio.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


extern Board *board;
SDL_Surface *screen;


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
			board_change_next_block(board);
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
		case SDLK_f:
			if (SDL_WM_ToggleFullScreen(screen) == 0) {
				fprintf(stderr, "Unable to toggle fullscreen/windowed mode.\n");
				exit(-1);
			}
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



/**
 * Create a delay of 'sec' seconds that can be cancelled by hitting any key
 */
int
cancellable_delay(int sec)
{
	int i;
	SDL_Event event;

	for (i = 0; i < sec * 100; i++) {
		if (SDL_PollEvent(&event) && event.type == SDL_KEYDOWN)
			return 1;

		SDL_Delay(10);
	}

	return 0;
}


