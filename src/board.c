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

#include "SDL.h"

#include "rezerwar.h"


extern Configuration *conf;
extern Board *board;
extern SDL_Surface *screen;
extern SDL_Surface *sprites;
extern Uint32 key;

/* Colors of the text when displaying "4x combo!" */
int combo_colors[(MAX_COMBO - 1) * 6] = {
	/* foreground		border */
	255, 255,   0,		120, 120,   0,	// 2x
	255, 120,   0,		120,  60,   0,
	255,   0,   0,		120,   0,   0,
	255,   0, 120,		120,   0,  60,	// 5x
	255,   0, 255,		120,   0, 120,
	255, 120, 255,		120,  60, 120,
	255, 255, 255,		120, 120, 120,
	120, 255, 120,		 60, 120,  60,
	  0, 255,   0,		  0, 120,   0,	// 10x
};


/**
 * @constructor
 */
Board *
board_new()
{
	Board *b;
	size_t size = BOARD_WIDTH * BOARD_HEIGHT;
	char *path;
	int i;

	b = r_malloc(sizeof(Board));

	b->width = BOARD_WIDTH;
	b->height = BOARD_HEIGHT;

	b->offset_x = BOARD_LEFT;
	b->offset_y = BOARD_TOP;

	b->bg = NULL;

	b->elapsed = 0;

	/* Cube related members initialization. */
	b->cube_count = 0;
	b->cubes = r_malloc(size * sizeof(Cube *));
	for (i = 0; i < size; i++)
		b->cubes[i] = NULL;

	/* Initialize the Cube Queue */
	b->cqueue = NULL;
	b->cqueue_len = 0;

	/* Cube related members. */
	b->current_cube = NULL;
	b->hold = NULL;
	b->next_cube = NULL;
	b->remains = -1;
	b->launch_next = false;
	b->next_line = 1;
	b->time_limit = -1;

	/* Mole related members */
	b->last_mole = -1;
	for (i = 0; i < MAX_MOLES; i++)
		b->moles[i] = NULL;

	/* Initial flames initialization */
	board_initialize_flames(b);

	/* Pipe status */
	for (i = 0; i < BOARD_HEIGHT * 2; i++)
		b->pipes[i] = pipe_new();

	/* Texts (future OSD) related members */
	b->texts = NULL;
	b->text_count = 0;

	/* Movement related (& controls) */
	b->moving_left = 0;
	b->moving_right = 0;
	b->lateral_speed = 100;
	b->lateral_tick = 0;

	/* Player related */
	b->score = 0;
	b->settled = false;
	b->combo = 0;
	b->status = MTYPE_NOP;
	b->paused = false;
	b->gameover = false;
	b->success = false;
	b->silent = false;
	b->allow_bomb = true;
	b->allow_medic = true;
	b->objective_type = OBJTYPE_NONE;
	board_set_difficulty_from_score(b);

	/* Prompt init. */
	b->prompt_text = NULL;
	b->prompt_func = NULL;

	/* Modal and score */
	b->modal = false;
	b->score_t = board_add_text(b, "0", 10, 10);
	b->timeleft_t = board_add_text(b, "", 10, 30);

	/* Status message */
	b->status_t = board_add_text(b, "", 0, 240);
	b->status_t->effect = EFFECT_SHAKE;
	b->status_t->centered = true;
	text_set_color1(b->status_t, 225, 186, 0);
	text_set_color2(b->status_t, 127,  55, 0);

	/* (optional) FPS display */
	b->show_fps = false;
	b->fps_t = board_add_text(b, "", 550, 10);
	text_set_color1(b->status_t, 225, 40, 0);
	text_set_color2(b->status_t, 56,  8, 8);

	/* Load background. */
	path = dpath("gfx/gameback.bmp");
	b->bg = SDL_LoadBMP(path);
	if (b->bg == NULL)
		fatal("Unable to load game background.");
	r_free(path);
	SDL_SetColorKey(b->bg, SDL_SRCCOLORKEY|SDL_RLEACCEL, 
			SDL_MapRGB(b->bg->format, 0, 255, 255));

	/* Level stuff */
	b->next_level = NULL;

	return b;
}


/**
 * Create a new board and populate the cubes from the given level.
 */
Board *
board_new_from_level(Level *level)
{
	int i;
	Board *board;
	Cube *cube = NULL;
	Text *title, *description, *prompt;

	board = board_new();

	/* Transfer the cubes */
	for (i = 0; i < (BOARD_WIDTH * BOARD_HEIGHT); i++) {
		cube = cube_new_from_char(level->cmap[i]);
		if (cube == NULL)
			continue;
		cube->y = i / BOARD_WIDTH;
		cube->x = i % BOARD_WIDTH;
		cube_sync_map(board, cube);
	
		/* No need to count rocks.. their passive. */
		if (cube->type == CTYPE_ROCK)
			continue;

		board->cube_count++;
	}

	/* Transfer the queue */
	board->cqueue_len = level->queue_len;
	board->cqueue = r_malloc(sizeof(Cube *) * board->cqueue_len);
	for (i = 0; i < board->cqueue_len; i++) {
		cube = cube_new_from_char(level->queue[i]->cmap[0]);
		cube->speed = board->cube_speed;
		board->cqueue[i] = cube;
	}

	/* Prepare the board to welcome the text */
	board->modal = true;
	board->silent = true;
	board->paused = true;

	/* Copy level related stuff */
	board->objective_type = level->objective_type;
	board->allow_bomb = level->allow_bomb;
	board->allow_medic = level->allow_medic;
	if (level->next)
		board->next_level = r_strcp(level->next);
	if (level->max_cubes)
		board->remains = level->max_cubes;
	board->rising_speed = level->rising_speed;
	board->time_limit = level->time_limit;
	board->max_moles = level->max_moles;

	/* Break a couple pipes at the bottom */
	for (i = 0; i < level->dead_pipes; i++) {
		board->pipes[BOARD_HEIGHT - i - 1]->status = 1;
		board->pipes[BOARD_HEIGHT * 2 - i - 1]->status = 1;
	}

	/* Draw the title */
	title = board_add_text(board, level->name, 20, 20);
	title->centered = true;
	title->temp = true;
	text_set_colors(title, 0xFFE64B, 0xB35904);

	/* Draw the description */
	description = board_add_text(board, level->description, 20, 90);
	description->temp = true;
	description->font = 1;

	/* Draw the press p to contine */
	prompt = board_add_text(board, "press 'enter' to start", 350, 440);
	prompt->temp = true;
	text_set_colors(prompt, 0xFFE64B, 0xB35904);

	return board;
}

void
board_kill(Board *board)
{
	int i, size;

	/* Cube cleanup (only if cubes we have) */
	if (board->cubes != NULL) {
		size = board->width * board->height;
		for (i = 0; i < size; i++) {
			if (board->cubes[i] == NULL)
				continue;

			cube_kill(board->cubes[i]);
		}
		r_free(board->cubes);
		board->cubes = NULL;
	}


	/* Cube clean up */
	if (board->next_cube != NULL) {
		cube_kill(board->next_cube);
		board->next_cube = NULL;
	}

	/* Text cleanup */
	for (i = 0; i < board->text_count; i++) {
		if (board->texts[i] == NULL)
			continue;
		text_kill(board->texts[i]);
	}
	free(board->texts);

	/* Cube queue cleanup */
	r_free(board->cqueue);
	board->cqueue = NULL;
	board->cqueue_len = 0;

	/* Mole cleanup */
	for (i = 0; i < MAX_MOLES; i++) {
		if (board->moles[i] == NULL)
			continue;

		mole_kill(board->moles[i]);
	}

	/* Pipe cleanup */
	for (i = 0; i < BOARD_HEIGHT * 2; i++) {
		pipe_kill(board->pipes[i]);
	}

	/* Level stuff */
	r_free(board->next_level);

	/* General board clean up */
	SDL_FreeSurface(board->bg);
	r_free(board);
}

Text *
board_add_text(Board *board, char *value, int x, int y)
{
	Text *t;

	t = text_new(value);
	
	board->texts = realloc(board->texts, (board->text_count + 1) * sizeof(Text*));
	board->texts[board->text_count] = t;
	board->text_count++;

	t->x = x;
	t->y = y;

	return t;
}

/**
 * Render the score text on the top left corner of the screen.
 */
#define MAX_SCORESTR_LEN	20
void
board_render_texts_score(Board *board)
{
	char score[MAX_SCORESTR_LEN] = "";

	/* 
	 * If the board is not in silent mode and is not in tutorial mode
	 * (i.e. with objective), then draw the score, we are in normal mode.
	 */
	if (board->silent != true && board->objective_type == OBJTYPE_NONE) {
		snprintf(score, MAX_SCORESTR_LEN, "score: %d", board->score);
	}

	text_set_value(board->score_t, score);
}

/**
 * Render all the texts of the board.
 */
void
board_render_texts(Board *board)
{
	int i;
	Text *t;
	SDL_Rect r;
	SDL_Surface *s;

	board_render_texts_score(board);

	/* Update the time Text */
	if (board->time_limit > -1) {
		char tl[20];
		snprintf((char *)tl, 20, "time: %d", board->time_limit);
		text_set_value(board->timeleft_t, tl);
	}

	/* Add a modal under all the text. */
	if (board->modal == true)
		gfx_modal(160);

	/* Draw all the Texts, cleaning up trashed ones. */
	for (i = 0; i < board->text_count; i++) {
		t = board->texts[i];
		if (t == NULL) continue;

		if (t->trashed == true) {
			text_kill(t);
			board->texts[i] = NULL;
			continue;
		}
		
		s = text_get_surface(t);

		/* It's legal for a text to have no surface, just skip it */
		if (s == NULL)
			continue;

		text_get_rectangle(t, &r);

		SDL_BlitSurface(s, NULL, screen, &r);
		SDL_FreeSurface(s);
	}
}

/**
 * Remove all the text marked 'temp'.
 */
void
board_trash_temp_texts(Board *board)
{
	Text *t;
	int i;

	for (i = 0; i < board->text_count; i++) {
		t = board->texts[i];
		if (t == NULL)
			continue;

		if (t->temp == true)
			t->trashed = true;
	}
}

/**
 * Toggle paused state. Ignore the request in GameOver mode.
 */
void
board_toggle_pause(Board *board)
{
	if (board->gameover)
		return;

	if (board->paused == true) {
		board->modal = false;
		board->paused = false;
		text_set_value(board->status_t, "");
		board_trash_temp_texts(board);
	} else {
		board->silent = false;
		board->modal = true;
		board->paused = true;
		text_set_value(board->status_t, "paused!");
	}
}

/**
 * Apply a mask on top of all the rendered shit
 */
void
board_render_transition(Board *board)
{
	switch (board->transition) {
		case TTYPE_SHUTTER_OPEN:
			gfx_shutter_open();
			board->transition = TTYPE_NONE;
			break;
		/*
		case TTYPE_PIXEL_OPEN:
			gfx_pixel_open();
			board->transition = TTYPE_NONE;
			break;
		case TTYPE_GREY_CURTAIN:
			gfx_transition_grey_curtain();
			board->transition = TTYPE_NONE;
			break;
		*/
		case TTYPE_NONE:
		default:
			break;
	}
}


/**
 * Render the moles.
 */
void
board_render_moles(Board *board)
{
	int i;

	for (i = 0; i < MAX_MOLES; i++) {
		if (board->moles[i] == NULL)
			continue;

		mole_render_trail(board->moles[i]);
	}

	for (i = 0; i < MAX_MOLES; i++) {
		if (board->moles[i] == NULL)
			continue;

		mole_render(board->moles[i]);
	}
}


/**
 * Main render function, actually dump pixels on the screen.
 */
void
board_render(Board *board)
{
	/* Redraw the sky. */
	sky_render(board);

	/* Redraw the background. */
	SDL_BlitSurface(board->bg, NULL, screen, NULL);

	/* Redraw the next and hold cubes. */
	board_render_next(board);
	board_render_hold(board);

	/* Redraw all the cubes. */
	board_render_cubes(board);

	/* Render all the explosions/flames */
	board_render_flames(board);

	/* Moles */
	board_render_moles(board);

	/* Pipe statuses */
	board_render_pipes(board);

	/* Animations */
	chimneys_render(board);

	/* Draw texts elements (meant to replace OSD), and modal */
	board_render_texts(board);

	/* Apply the transition if any */
	board_render_transition(board);

	/* Dig up the back buffer. */
	SDL_Flip(screen);
}


/**
 * Called when the game has ended, return the appropriate MTYPE.
 */
enum mtype
board_gameover(Board *board)
{
	/* Success is only for tutorial mode. */
	if (board->success) {
		if (board->next_level) {
			r_free(conf->next_level);
			conf->next_level = r_strcp(board->next_level);
		}
		board_render(board);
		return MTYPE_GAMEOVER_WIN;
	}

	/* The player made it without using hold, extra pts! */
	if (board->hold == NULL)
		board->score += POINTS_NO_HOLD;

	/* The player made a hiscore! */
	if (hiscore_check(board->score) == true) {
		conf->last_score = board->score;
		return MTYPE_GAMEOVER_HISCORE;
	}

	/* The player lost and didn't make a hiscore */
	return MTYPE_GAMEOVER_LOSE;
}


void
board_update_pipes(Board *board, uint32_t now)
{
	Pipe *pipe;
	int i;

#define PIPE_ANIM_SPEED	100

	for (i = 1; i < BOARD_HEIGHT * 2; i++) {
		pipe = board->pipes[i];

		if (pipe->tick + PIPE_ANIM_SPEED > now)
			continue;

		if (pipe->status != -1) {
			if (pipe->status < 10) { // b0rked pipe
				if (pipe->status >= 3)
					pipe->status = 0;
				else {
					pipe->status++;
				}
			}
		}
		pipe->tick = now;
	}
}


/**
 * This function handles all the elements at ticking point, if anything
 * pushed the board to a game over, let the caller know with an mtype.
 */
enum mtype
board_update(Board *board, uint32_t now)
{
	/*
	 * The game is in pause, this means no internal logic runs, nothing
	 * is happening, not even teh 'elapsed' time flows.
	 */
	if (board->paused == true)
		return MTYPE_NOP;

	/*
	 * Every seconds elapsed, decrement the amount of second left from
	 * the time limit.
	 */
	board->elapsed += TICK;
	if (board->elapsed > 1000) {
		board->elapsed = 0;
		board->time_limit--;
	}

	/*
	 * If a time limit is set on this level and we've reached it, return
	 * with an MTYPE stating so.
	 */
	if (board->time_limit == 0) {
		board->gameover = true;
		return MTYPE_GAMEOVER_TIMEOUT;
	}

	/*
	 * Update all the game logic components
	 */
	board_update_cubes(board, now);
	board_update_flames(board, now);
	board_update_water(board, now);
	board_update_moles(board, now);
	board_update_pipes(board, now);

	/* Check the cube count for CLEARALL levels. */
	if (board->objective_type == OBJTYPE_CLEARALL && 
			board->cube_count == 0) {
		board->gameover = true;
		board->success = true;
	}

	/* Stop here, something in the 'update' caused a gameover */
	if (board->gameover == true)
		return board_gameover(board);

	/* We need a new line! */
	if (board->rising_speed > -1 && board->next_line <= now) {
		if (board->next_line != 1) {
			board_add_line(board);
			/* Delay the current cube's tick, looks better */
			board->current_cube->tick = now;
			board->current_cube->prev_y--;
		}
		board->next_line = now + board->rising_speed * 1000;
	}

	/* We were requested to launch the next cube */
	if (board->launch_next == true) {
		board_launch_next_cube(board);
		board->launch_next = false;
	}

	/* Animations */
	chimneys_update(board, now);
	sky_update(board, now);

	return MTYPE_NOP;
}


/**
 * Generate a random line of cube at the bottom and move everything up one
 * cube. Do whatever you can to avoid self-triggering lines.
 */
void
board_add_line(Board *board)
{
	Cube	*cube;
	int	 size = board->width * board->height;
	int	 i;

	/* Raise all the cubes of the board of one */
	for (i = 0; i < size; i++) {
		cube = board->cubes[i];

		if (cube == NULL)
			continue;

		if (i < board->width) {
			board->gameover = true;
			continue;
		}

		cube->y--;

		if (cube->y < 0)
			board->gameover = true;

		cube_sync_map(board, cube);
	}

	/* Populate this new free line with random cubes */
	board_prepopulate(board, 1);
}


/**
 * Load a new random cube or follow the queue depending on the type of
 * game.
 */
void
board_load_next_cube(Board *board)
{
	if (board->cqueue_len) {
		board->next_cube = board->cqueue[board->cqueue_len - 1];
		board->cqueue[board->cqueue_len - 1] = NULL;
		board->cqueue_len--;
	} else {
		board->next_cube = cube_new_one(board->allow_bomb,
				board->allow_medic);
	}

	board->next_cube->speed = board->cube_speed;
	board->next_cube->falling = true;
}


/**
 * Take whatever cube is currently loaded as next_cube, add it to the
 * list of cubes for this board, place it at the right place and pick a
 * new next_cube.
 */
void
board_launch_next_cube(Board *board)
{
	Cube *cube = board->next_cube;
	int i;

	/* If we had 0 remaining cubes... you lost. */
	if (board->remains == 0) {
		board->gameover = true;
		return;
	}

	/* Load the new cube */
	board->current_cube = cube;
	cube->x = (board->width - 1) / 2;
	cube->y = 0;
	board->cube_count++;
	i = cube->x + cube->y * board->width;

	/* We already have someone there, you lose. */
	if (board->cubes[i] != NULL) {
		board->gameover = true;
		return;
	}

	/* Position the cube on the board */
	cube_sync_map(board, cube);

	/* Decrement the remaining cubes */
	if (board->remains > 0)
		board->remains--;

	/* If we are NOW at 0, you are on your last cube */
	if (board->remains == 0) {
		board->next_cube = NULL;
		return;
	}

	board_load_next_cube(board);
}



void
board_change_next_cube(Board *board)
{
	cube_kill(board->next_cube);
	board_load_next_cube(board);
}



/**
 * Handle the rendering of the top "next" cube.
 */
void
board_render_next(Board *board)
{
	SDL_Surface	*surface;
	Cube		*cube = board->next_cube;

	if (cube == NULL)
		return;

	surface = cube_get_surface(cube);

	gfx_toscreen(surface, cube->x * BSIZE + NEXT_CUBE_LEFT + BSIZE / 2,
			      cube->y * BSIZE + NEXT_CUBE_TOP + BSIZE / 2);
	gfx_free(surface);
}


/**
 * Handle the rendering of the top "hold" cube.
 */
void
board_render_hold(Board *board)
{
	SDL_Surface	*surface;
	Cube		*cube = board->hold;

	if (cube == NULL)
		return;

	surface = cube_get_surface(cube);

	gfx_toscreen(surface, HOLD_LEFT + BSIZE / 2, HOLD_TOP + BSIZE / 2);
	gfx_free(surface);
}

/**
 * Transfer all the cubes from a cube to the board. This is anticipating the
 * death of a cube. Set the cube_count to 0 to avoid duplicate killing. Also
 * checks for special cubes and execute them.
 */
void
cube_landing(Board *board, Cube *cube)
{
	/*
	 * This will let other functions in this loop know that a cube
	 * reach its final destination, it is reset to false at every
	 * new loops.
	 */
	if (board->current_cube == cube) {
		board->settled = true;
		board->launch_next = true;
	}

	cube->falling = false;

	/* Trigger special cubes */
	switch (cube->type) {
		case CTYPE_BOMB:
			board_cube_bomb(board, cube);
			break;
		case CTYPE_MEDIC:
			board_cube_medic(board, cube);
			/* !FALLTHROUGH! TODO: find medic sound */
		default:
			sfx_play_tack1();
			break;
	}
}


void
board_kill_row(Board *board, int row)
{
	int i;

	for (i = board->width * row; i < board->width * (row + 1); i++) {
		if (board->cubes[i] == NULL)
			continue;
		board_trash_cube(board, board->cubes[i]);
	}
}


void
board_kill_column(Board *board, int col)
{
	int i;

	for (i = col; i < board->width * board->height; i += board->width) {
		if (board->cubes[i] == NULL)
			continue;
		board_trash_cube(board, board->cubes[i]);
	}
}


/**
 * Easy wrapper to pass directly a cube.
 */
void
board_add_points_from_cube(Board *board, int points, Cube *cube)
{
	board_add_points(board, points, cube_get_abs_x(cube) + 2,
			cube_get_abs_y(cube) + BSIZE / 2);
}


/**
 * Add new points to the score, displaying that on screen.
 */
void
board_add_points(Board *board, int points, int x, int y)
{
	Text	*text;
	char	 value[16];
	int	 xpts = points * (board->combo + 1);

	/* Don't bother counting score in tutorial mode */
	if (board->objective_type != OBJTYPE_NONE)
		return;

	board->score += xpts;

	snprintf(value, 16, "%d", xpts);

	text = board_add_text(board, value, x, y);
	text->effect |= EFFECT_FADEOUT | EFFECT_FLOAT;
}


/**
 * Load the combo color according to the array up there.
 */
void
board_get_combo_colors(Board *board, int *fr, int *fg, int *fb, int *br, int *bg, int *bb)
{
	int m = (board->combo - 2) * 6;

	*fr = combo_colors[m + 0];
	*fg = combo_colors[m + 1];
	*fb = combo_colors[m + 2];
	*br = combo_colors[m + 3];
	*bg = combo_colors[m + 4];
	*bb = combo_colors[m + 5];
}

/**
 * Show combos on screen as they get added.
 */
void
board_show_combo(Board *board)
{
	Text	*text;
	char	 buf[16];
	int	 fr, fg, fb,	// foreground color
		 br, bg, bb;	// border color

	if (board->combo < 2)
		return;

	board_get_combo_colors(board, &fr, &fg, &fb, &br, &bg, &bb);

	snprintf(buf, 16, "%dx combo!", board->combo);
	text = board_add_text(board, buf, 0, 200);
	text->centered = true;
	text->effect |= EFFECT_FADEOUT | EFFECT_SHAKE;
	text_set_color1(text, fr, fg, fb);
	text_set_color2(text, br, bg, bb);
}


/**
 * A Cube Medic just dropped! If a broken pipe is around, fix it. In any case,
 * it makes sure all the surrounding cubes are opened.
 */
#define MAKE_SIDE_CUBE_ALL(RX, RY) \
	side_cube = board_get_cube(board, cube->x + RX, cube->y + RY); \
	if (side_cube && side_cube->type != CTYPE_ROCK) \
		side_cube->type = CTYPE_ALL;
void
board_cube_medic(Board *board, Cube *cube)
{
	Pipe	*left_pipe = board->pipes[cube->y];
	Pipe	*right_pipe = board->pipes[cube->y + BOARD_HEIGHT];
	Cube	*side_cube;

	/* Medics shouldn't stop combos from happening, they are good things */
	board->settled = false;

	/* In front of a broken pipe on the left */
	if (cube->x == 0 && left_pipe->status != -1) {
		pipe_fix(board, left_pipe);

	/* In front of a broken pipe on the right */
	} else if (cube->x == (BOARD_WIDTH - 1) && right_pipe->status != -1) {
		pipe_fix(board, right_pipe);

	/*
	 * Find adjacent cubes and convert them so that they all have an 
	 * opening toward this new cube. For now, convert all of them into
	 * 'all-way' cubes.
	 */
	} else {
		/* Sides */
		MAKE_SIDE_CUBE_ALL( 0,  1);
		MAKE_SIDE_CUBE_ALL(-1,  0);
		MAKE_SIDE_CUBE_ALL( 0, -1);
		MAKE_SIDE_CUBE_ALL( 1,  0);
	}

	board_trash_cube(board, cube);
	sfx_play_menuselect();
}


/**
 * Set the speed of cubes, the number of moles, relative to the score.
 */
void
board_set_difficulty_from_score(Board *board)
{
	if (board->score < 5000) {
		board->cube_speed = 1000;
		board->max_moles = 2;
		board->rising_speed = 30;
	} else if (board->score < 10000) {
		board->cube_speed = 850;
		board->max_moles = 3;
		board->rising_speed = 25;
	} else if (board->score < 15000) {
		board->cube_speed = 800;
		board->max_moles = 4;
		board->rising_speed = 25;
	} else if (board->score < 20000) {
		board->cube_speed = 750;
		board->max_moles = 5;
		board->rising_speed = 20;
	} else if (board->score < 25000) {
		board->cube_speed = 700;
		board->max_moles = 6;
		board->rising_speed = 20;
	} else {
		board->cube_speed = 650;
		board->max_moles = 7;
		board->rising_speed = 15;
	}
}


/**
 * Update an individual cube during the current tick (which time is
 * reprensented by 'now'.
 */
void
board_update_falling_cube(Board *board, uint32_t now, Cube *cube) {
	/* This cube's tick has expired, we might need to move it. */
	if (now - cube->tick > cube->speed) {
		/* Can it fit one unit lower? */
		if (board_move_check(board, cube, 0, 1) == 0) {
			cube->y++;
			cube->tick = now;
			cube_sync_map(board, cube);
		}

		/* If the cube didn't move for a tick, it is landing. */
		else if (cube->prev_y == cube->y) {
			cube_landing(board, cube);
			return;
		}

		cube->prev_y = cube->y;
	}

	/* Ticking for the lateral moves. */
	if (now - board->lateral_tick > board->lateral_speed) {
		if (board->moving_left > 1) {
			board->moving_left--;
		} else if (board->moving_left == 1) {
			board_move_current_cube_left(board);
		}

		if (board->moving_right > 1) {
			board->moving_right--;
		} else if (board->moving_right == 1) {
			board_move_current_cube_right(board);
		}

		board->lateral_tick = now;
	}
}	


/**
 * move_check()
 * 	bside is the value telling if the cube is on the left of the whole
 * 		cube (1) or on the right side (2).
 *
 * Return values:
 * 	0 when path is clear
 * 	3 when touching the bottom
 * 	1 when a cube from the left side blocked
 * 	2 when a cube from the right side blocked
 * 	4 when another cube is in the way
 */
byte
board_move_check(Board *board, Cube *cube, Sint8 x, Sint8 y)
{
	int	j;

	/* Reached the bottom of the board. */
	if (cube->y + y >= board->height)
		return 3;

	/* Reached the left border. */
	if (cube->x + x < 0)
		return 1;

	/* Reach the right border. */
	if (cube->x + x >= board->width)
		return 2;

	/* There is a cube in this direction. */
	j = (cube->y + y) * board->width + (cube->x + x);
	if (board->cubes[j]) {
		return 1;
	}

	return 0;
}


void
board_move_current_cube_left(Board *board)
{
	Cube *cube = board->current_cube;

	if (cube == NULL)
		return;

	/* Don't rotate cube during pause. */
	if (board->paused == true)
		return;

	if (board_move_check(board, cube, -1, 0) == 0) {
		cube->x--;
		cube_sync_map(board, cube);
	}
}


void
board_move_current_cube_right(Board *board)
{
	Cube *cube = board->current_cube;

	if (cube == NULL)
		return;

	/* Don't rotate cube during pause. */
	if (board->paused == true)
		return;

	if (board_move_check(board, cube, 1, 0) == 0) {
		cube->x++;
		cube_sync_map(board, cube);
	}
}


/**
 * In charge of rotating a cube on a board. If there is no space for a
 * rotation, ignore, if against a cube, move a bit. If cube is NULL,
 * default to current.
 */
void
board_rotate_cw(Board *board)
{
	Cube *cube = board->current_cube;

	/* Don't rotate cube during pause. */
	if (board->paused == true)
		return;

	cube_rotate_cw(cube);
	sfx_play_tick1();
}


/**
 * Update all the cubes logic (non-graphic stuff).
 */
void
board_update_cubes(Board *board, uint32_t now)
{
	int i;
	int size = board->width * board->height;
	int type;
	Cube *cube;

	board->settled = false;

	/* Only re-adjust the difficulty in normal mode */
	if (board->objective_type == OBJTYPE_NONE)
		board_set_difficulty_from_score(board);

	for (i = 0; i < size; i++) {
		cube = board->cubes[i];

		if (cube == NULL)
			continue;

		/* Check if the cube is trashed (if yes fade it to death) */
		if (cube->trashed == true) {
			cube->fade_status++;
			if (cube->fade_status > BSIZE / 2) {
				cube_kill(cube);
				board->cubes[i] = NULL;
			}
			continue;
		}

		/* Falling cube have their own logic */
		if (cube->falling == true) {
			board_update_falling_cube(board, now, cube);
			continue;
		}


		/* If the cube is a rock, just skip, rocks don't move */
		if (cube->type == CTYPE_ROCK)
			continue;

		/* 
		 * Check if the cube has free space under itself, if yes 
		 * disconnect it as a cube and create a new cube falling
		 * much faster.
		 */
		type = board_get_area_type(board, cube->x, cube->y + 1);
		if (type == ATYPE_FREE) {
			cube->falling = true;
			cube->speed = 100;
		}
	}
}


/**
 * Rendering the cubes is actually handling the graphic part, i.e. blitting
 * the textures at the right place.
 */
void
board_render_cubes(Board *board)
{
	int		 i;
	int		 size = board->width * board->height;
	SDL_Surface	*surface;
	Cube		*cube;

	for (i = 0; i < size; i++) {
		cube = board->cubes[i];

		if (cube == NULL)
			continue;

		surface = cube_get_surface(cube);

		gfx_toscreen(surface, cube->x * BSIZE + BOARD_LEFT,
				      cube->y * BSIZE + BOARD_TOP);
		gfx_free(surface);
	}
}


/**
 * Dump on stdout the map of the cube as it stands right now.
 */
void
board_dump_cube_map(Board *board)
{
	byte x, y;
	int i;

	for (y = 0; y < board->height; y++) {
		for (x = 0; x < board->width; x++) {
			i = y * board->width + x;
			if (board->cubes[i]) {
				printf("x");
			} else {
				printf(" ");
			}
		}
		printf("\n");
	}
}


/**
 * Go through all the cubes and remove the water, untie all the current
 * networks.
 */
void
board_remove_water(Board *board)
{
	int i;
	int bs = board->width * board->height;

	/* Get rid of all the water. */
	for (i = 0; i < bs; i++) {
		if (board->cubes[i] == NULL)
			continue;

		board->cubes[i]->water = 0;
		board->cubes[i]->root = NULL;
		board->cubes[i]->network_integrity = 1;
		cube_network_flush(board->cubes[i]);
	}
}


/**
 * Check the left and right side for cubes. If any, check if they have opened
 * pipes on the same side.
 */
void
board_update_water(Board *board, uint32_t now)
{
	int	i,			// loop helper
		combo = board->combo;	// backup the combo
	Cube	*cube;			// current cube during the loops

	board_remove_water(board);

	/* Scan the left side... */
	for (i = 0; i < board->height; i++) {
		cube = board->cubes[i * board->width];
		if (cube == NULL || cube->falling == true)
			continue;

		if (board->pipes[i]->status != -1)
			continue;

		if (cube_plug_match(cube, PLUG_WEST))
			board_spread_water(board, cube, NULL, 1);
	}

	/* 
	 * Now while scanning the right side, also check if water made it all
	 * the way through, in this case, taint the network.
	 */
	for (i = 0; i < board->height; i++) {
		cube = board->cubes[(i + 1) * board->width - 1];
		if (cube == NULL || cube->falling == true)
			continue;

		if (board->pipes[BOARD_HEIGHT+i]->status != -1)
			continue;

		if (cube_plug_match(cube, PLUG_EAST)) {
			if (cube->water == 1) {
				cube_network_taint(cube->root);
				/* If we are in link mode, this is a win */
				if (board->objective_type == OBJTYPE_LINK) {
					board->gameover = true;
					board->success = true;
				}
			} else
				board_spread_water(board, cube, NULL, 2);
		}
	}

	/*
	 * Now we know what networks are tainted, go through the left side
	 * again and look for a root cube (net length > 1), with red water (3),
	 * and a network_integrity preserved. Toggle an avalanche if found.
	 */
	for (i = 0; i < board->height; i++) {
		cube = board->cubes[i * board->width];
		if (cube == NULL || cube->falling == true)
			continue;

		if (cube->network_size > 1 && cube->water == 3 &&
				cube->network_integrity == 1) {
			if (cube->fade_status > 0) continue;
			board_run_avalanche(board, cube);
		}
	}

	/*
	 * If we are in a loop with a newly settled cube (reach its final
	 * destination), check if the user has completed a new network (if
	 * board->combo has changed). If no, reset it to zero.
	 */
	if (board->settled == true) {
		if (combo == board->combo) {
			board->combo = 0;
		} else {
			if (board->combo > MAX_COMBO)
				board->combo = MAX_COMBO;
			board_show_combo(board);
		}
	}
}


/**
 * Run avalanche on the specific cube. This means the cube was already
 * identified as the root of a tainted network.
 */
void
board_run_avalanche(Board *board, Cube *cube)
{
	int	 i;
	Text	*text;

	/* Start a fading text... */
	text = board_add_text(board, "Excellent!", 240, 200);
	text->centered = true;
	text->temp = true;
	text_set_color1(text, 255, 0, 0);
	text_set_color2(text, 80, 0, 0);
	text->effect |= EFFECT_SHAKE | EFFECT_FADEOUT;

	board_add_points_from_cube(board, POINTS_AVALANCHE, cube);

	/* Run each columns individually */
	board_run_avalanche_column(board, cube);
	for (i = 0; i < cube->network_size; i++) {
		board_run_avalanche_column(board, cube->network[i]);
	}
}


/**
 * Run an avalanche on only one column, starting from the cube
 */
void
board_run_avalanche_column(Board *board, Cube *cube)
{
	int y;
	Cube *target;

	for (y = cube->y; y < board->height; y++) {
		target = board_get_cube(board, cube->x, y);

		if (target->type == CTYPE_ROCK)
			continue;

		if (target != NULL && target->trashed == false) {
			board_trash_cube(board, target);
			board_add_points_from_cube(board, POINTS_NETWORK_FACTOR,
					target);
		}
	}
}


/**
 * Return a cube at the given coordinates, return NULL if not found or if
 * border of the board.
 */
Cube *
board_get_cube(Board *board, int x, int y)
{
	/* Bad x value */
	if (x < 0 || x >= board->width)
		return NULL;

	/* Bad y value */
	if (y < 0 || y >= board->height)
		return NULL;

	return (board->cubes[x + board->width * y]);
}


/**
 * Return a cube at the given screen coordinates, return NULL if nothing
 * found.
 */
Cube *
board_get_cube_absolute(Board *board, int x, int y)
{
	int rx, ry;

	rx = (x - BOARD_LEFT) / BSIZE;
	ry = (y - BOARD_TOP) / BSIZE;

	return board_get_cube(board, rx, ry);
}


/**
 * Returns an integer representing the type of area at x,y
 */
int
board_get_area_type(Board *board, int x, int y)
{
	if (x < 0)
		return ATYPE_BOARD_LEFT;

	if (x >= board->width)
		return ATYPE_BOARD_RIGHT;

	if (y >= board->height)
		return ATYPE_BOARD_BOTTOM;

	if (board->cubes[x + board->width * y] != NULL)
		return ATYPE_CUBE;

	return ATYPE_FREE;
}


/**
 * Given specific x/y offsets, try to spread the water to a neighboring cube
 */
void
board_spread_attempt(Board *board, Cube *cube, Cube *root, Sint8 ox, Sint8 oy,
		byte src_plug, byte dest_plug)
{
	Cube	*n;
	int	 status;
	int	 type;

	type = board_get_area_type(board, cube->x + ox, cube->y + oy);
	switch (type) {
		case ATYPE_FREE:
			if (cube_plug_match(cube, src_plug))
				root->network_integrity = 0;
			break;
		case ATYPE_CUBE:
			n = board_get_cube(board, cube->x + ox, cube->y + oy);

			/* Falling blocks should not be part of any network */
			if (n->falling && cube_plug_match(cube, src_plug)) {
				root->network_integrity = 0;
				return;
			}

			status = cube_get_plug_status(cube, src_plug, n, 
					dest_plug);
			if (status == PSTAT_CONNECTED)
				board_spread_water(board, n, root, root->water);
			break;
		default:
			break;
	}
}


/**
 * A network is going to be destroyed.
 */
void
board_destroy_network(Board *board, Cube *cube)
{
	int i;

	if (cube->trashed == true)
		return;

	for (i = 0; i < cube->network_size; i++) {
		board_trash_cube(board, cube->network[i]);
	}

	/* Only count combos for networks with more than one cubes. */
	if (cube->network_size > 0)
		board->combo++;

	board_trash_cube(board, cube);
	board_add_points_from_cube(board, 
			(cube->network_size + 1) * POINTS_NETWORK_FACTOR,
			cube);

	sfx_play_lazer();
}


/**
 * Mark a cube for deletion (trashed), it will fade and ultimately be removed
 */
void
board_trash_cube(Board *board, Cube *cube)
{
	cube->trashed = true;
	cube_sync_map(board, cube);
	board->cube_count--;
}


/**
 * Take a cube and check its neighbors for propagation. 'n' is always the
 * neighbor cube.
 */
void
board_spread_water(Board *board, Cube *cube, Cube *root, int water_type)
{
	/* Don't continue if this cube is already watered. */
	if (cube->water >= 1)
		return;

	/* Add 'cube' to the network, it doesn't seem to be a starting point. */
	if (root == NULL) {
		root = cube;
	} else {
		cube_network_add(root, cube);
	}

	cube->water = water_type;

	/* North, East, South, West */
	board_spread_attempt(board, cube, root,  0, -1, PLUG_NORTH, PLUG_SOUTH);
	board_spread_attempt(board, cube, root,  1,  0, PLUG_EAST,  PLUG_WEST);
	board_spread_attempt(board, cube, root,  0,  1, PLUG_SOUTH, PLUG_NORTH);
	board_spread_attempt(board, cube, root, -1,  0, PLUG_WEST,  PLUG_EAST);

	if (root == cube && cube->network_integrity == 1) {
		board_destroy_network(board, cube);
	}
}


/**
 * Prepopulate a number of lines at the bottom of the board.
 */
void
board_prepopulate(Board *board, int lines)
{
	int x, y;
	Cube *cube;

	for (x = 1; x < board->width - 1; x++) {
		for (y = board->height - lines; y < board->height; y++) {
			cube = cube_new_random();
			cube->x = x;
			cube->y = y;
			cube_sync_map(board, cube);
			board->cube_count++;
		}
	}
}


/**
 * Keep the current cube aside. If a cube is currenly held, replace with
 * the current one.
 */
void
board_hold(Board *board)
{
	Cube *cube;

	if (board->hold == NULL) {
		board->hold = board->current_cube;
		board->hold->falling = false;
		board->launch_next = true;
		cube_sync_map(board, board->hold);
	} else {
		cube = board->hold;
		cube->x = board->current_cube->x;
		cube->y = board->current_cube->y;
		cube->prev_y = board->current_cube->prev_y;
		cube->tick = board->current_cube->tick;
		cube->falling = true;
		board->hold = board->current_cube;
		cube_sync_map(board, board->hold);
		board->current_cube = cube;
		cube_sync_map(board, cube);
	}
}


/**
 * Move the current cube as low as possible and trigger the cube transfer to
 * avoid the player to move it.
 */
void
board_cube_fall(Board *board)
{
	int offset = 1;
	Cube *cube = board->current_cube;

	while (board_move_check(board, cube, 0, offset) == 0)
		offset++;

	cube->y += offset - 1;
	cube->prev_y = cube->y;
	cube->tick += cube->speed * 2;

	cube_sync_map(board, cube);
}


/**
 * Generate a new mole. If reaching MAX, just remove the oldest.
 */
void
board_spawn_mole(Board *board)
{
	int idx = board->last_mole;

	if (idx >= MAX_MOLES - 1)
		idx = 0;
	else
		idx++;

	board->last_mole = idx;

	if (board->moles[idx] != NULL)
		mole_kill(board->moles[idx]);

	board->moles[idx] = mole_new();
	board->moles[idx]->board = board;
}

void
board_update_moles(Board *board, uint32_t now)
{
	int	i,			// loop variable
		alive_moles = 0;	// number of moles currently active


	for (i = 0; i < MAX_MOLES; i++) {
		if (board->moles[i] == NULL)
			continue;

		if (board->moles[i]->trashed == true) {
			mole_kill(board->moles[i]);
			board->moles[i] = NULL;
			continue;
		}

		alive_moles++;
		mole_update(board->moles[i], now);
	}

	/* Ensure we always have enough moles on screen. */
	for (i = alive_moles; i < board->max_moles; i++) {
		board_spawn_mole(board);
	}
}

