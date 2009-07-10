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


/*
 * Event handling during game play.
 */


#include <stdio.h>

#include "SDL.h"
#include "rezerwar.h"


extern Board *board;
extern Configuration *conf;
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
		if (event.type == SDL_JOYBUTTONDOWN) {
			text_set_value(prompt, "wiiuser");
			done = true;
		}
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
	enum { 
		nothing, 
		menu,
		fps,
		rotate_cw,
		pause,
		left,
		right,
		leftonce,
		rightonce,
		down,
		center,
		fullscreen
	} action = nothing;

	if (event->type == SDL_JOYHATMOTION) {
		if (event->jhat.value & SDL_HAT_LEFT)
			action = leftonce;
		else if (event->jhat.value & SDL_HAT_RIGHT)
			action = rightonce;
	}

#ifdef __WII__
	if (event->type == SDL_JOYBUTTONDOWN) {
		switch (event->jbutton.button) {
		case WPAD_BUTTON_HOME:
			action = menu;
			break;
		case WPAD_BUTTON_PLUS:
		case WPAD_BUTTON_MINUS:
			action = pause;
			break;
		case WPAD_BUTTON_1:
		case WPAD_BUTTON_2:
			action = rotate_cw;
			break;
		case WPAD_BUTTON_B:
			action = fps;
			break;
		default:
			break;
		}
	}
#endif

	if (event->type == SDL_KEYDOWN) {
		switch ((int)event->key.keysym.sym) {
		case SDLK_ESCAPE:
		case SDLK_q:
			action = menu;
			break;
		case SDLK_F12:
			action = fps;
			break;
		case SDLK_LEFT:
		case SDLK_h:
			action = left;
			break;
		case SDLK_RIGHT:
		case SDLK_l:
			action = right;
			break;
		case SDLK_DOWN:
		case SDLK_j:
			action = down;
			break;
		case SDLK_a:
		case SDLK_SPACE:
			action = rotate_cw;
			break;
		case SDLK_p:
		case SDLK_RETURN:
			action = pause;
			break;
		case SDLK_f:
			action = fullscreen;
			break;
		default:
			break;
		}
	}

	switch (action) {
		case menu:
			return MTYPE_SUBMENU;;
			break;
		case fps:
			board->show_fps = !board->show_fps;
			break;
		case left:
			board_move_current_block_left(board);
			board_refresh(board);
			board->lateral_tick = 0;
			board->moving_left = 4;
			break;
		case right:
			board_move_current_block_right(board);
			board_refresh(board);
			board->lateral_tick = 0;
			board->moving_right = 4;
			break;
		case leftonce:
			board_move_current_block_left(board);
			board_refresh(board);
			break;
		case rightonce:
			board_move_current_block_right(board);
			board_refresh(board);
			break;
		case down:
			board->block_speed_factor = 10;
			break;
		case rotate_cw:
			board_rotate_cw(board, NULL);
			break;
		case pause:
			board_toggle_pause(board);
			break;
		case fullscreen:
			/* SDL on win32 is unable to toggle video mode... */
			conf->fullscreen = !conf->fullscreen;
#ifdef _WIN32
			init_gfx();
#else
			if (SDL_WM_ToggleFullScreen(screen) == 0)
				fatal("Unable to toggle fullscreen/windowed mode.");
#endif
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
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYHATMOTION:
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
			case SDL_JOYBUTTONDOWN:
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


