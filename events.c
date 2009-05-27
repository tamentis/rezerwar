/*
 * This file is about event handling during game play.
 */

#include <stdio.h>

#include "SDL.h"

#include "rezerwar.h"


extern Board *board;
SDL_Surface *screen;


/**
 * Keyboard up (stop moving one side or stop accelerating).
 */
enum mtype
handle_events_keyup(SDL_Event *event)
{
	switch ((int)event->key.keysym.sym) {
		case SDLK_DOWN:
		case SDLK_j:
			board->block_speed_factor = 1;
			break;
		case SDLK_RIGHT:
		case SDLK_l:
			board->moving_right = 0;
			break;
		case SDLK_LEFT:
		case SDLK_h:
			board->moving_left = 0;
			break;
		default:
			break;
	}

	return MTYPE_NOP;
}


/**
 * In prompt mode, capture all the characters from the keyboard until
 * return. Return true when return was entered.
 */
bool
handle_events_prompt(SDL_keysym keysym, Text *text)
{
	char ch;

	if ((keysym.unicode & 0xFF80) != 0)
		return false;

	if (keysym.sym == SDLK_BACKSPACE) {
		text_del_last_char(text);
		return false;
	}

	if (keysym.sym == SDLK_RETURN) {
		return true;
	}

	ch = keysym.unicode & 0x7F;
	if (isalnum(ch) != 0)
		text_add_char(text, ch);

	return false;
}


bool
prompt_polling(Text *prompt)
{
	SDL_Event event;
	bool done = false;

	while (SDL_PollEvent(&event)) {
		if (event.type != SDL_KEYDOWN) continue;
		done = handle_events_prompt(event.key.keysym, prompt);
	}

	return done;
}


/**
 * Keydown.
 */
enum mtype
handle_events_keydown(SDL_Event *event)
{
	switch ((int)event->key.keysym.sym) {
		case SDLK_ESCAPE:
		case SDLK_q:
			return MTYPE_SUBMENU;;
		case SDLK_F12:
			board->show_fps = !board->show_fps;
			break;
		case SDLK_F10:
			board_change_next_block(board);
			break;
		case SDLK_F7:
//			board_add_cube(board);
			board_dump_cube_map(board);
			break;
		case SDLK_LEFT:
		case SDLK_h:
			board_move_current_block_left(board);
			board_refresh(board);
			board->lateral_tick = 0;
			board->moving_left = 4;
			break;
		case SDLK_RIGHT:
		case SDLK_l:
			board_move_current_block_right(board);
			board_refresh(board);
			board->lateral_tick = 0;
			board->moving_right = 4;
			break;
		case SDLK_DOWN:
		case SDLK_j:
			board->block_speed_factor = 10;
			break;
		case SDLK_a:
		case SDLK_SPACE:
			board_rotate_cw(board, NULL);
			break;
		case SDLK_p:
		case SDLK_RETURN:
			board_toggle_pause(board);
			break;
		case SDLK_f:
			if (SDL_WM_ToggleFullScreen(screen) == 0)
				fatal("Unable to toggle fullscreen/windowed mode.");
			break;
		default:
			break;
	}

	return MTYPE_NOP;
}


/**
 * Usually returns NOP unless SQL_QUIT (window close) is triggered, it gets
 * its mtypes from the sub-routines.
 */
enum mtype
handle_events(SDL_Event *event)
{
	switch (event->type) {
		case SDL_KEYDOWN:
			return handle_events_keydown(event);
			break;
		case SDL_KEYUP:
			return handle_events_keyup(event);
			break;
		case SDL_QUIT:
			return MTYPE_QUIT;
			break;
		default:
			break;
	}

	return MTYPE_NOP;
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


