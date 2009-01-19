#include <stdio.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


extern SDL_Surface *screen;


Board *
board_new(Uint8 width, Uint8 height, int difficulty)
{
	Board *b;
	size_t size = width * height;
	Uint16 i;

	b = r_malloc(sizeof(Board));

	b->width = width;
	b->height = height;

	b->difficulty = difficulty;

	b->offset_x = BOARD_LEFT;
	b->offset_y = BOARD_TOP;

	b->bg = NULL;

	/* Cube related members initialization. */
	b->cubes = r_malloc(size * sizeof(Cube *));
	for (i = 0; i < size; i++)
		b->cubes[i] = NULL;

	/* Block related members. */
	b->block_count = 0;
	b->blocks = NULL;
	b->current_block = NULL;
	b->next_block = NULL;
	b->block_speed = SPEED_NORMAL;

	/* Drop related members. */
	b->drop_map_size = size * BSIZE * BSIZE;
	b->drop_map = r_malloc(b->drop_map_size * sizeof(Drop*));
	for (i = 0; i < size; i++) {
		b->drop_map[i] = NULL;
	}

	b->drop_count = 0;
	b->drops = NULL;
	b->drop_speed = DROP_SPEED;

	/* Water output related members. */
	b->outputs = NULL;
	b->output_count = 0;

	/* text_ts (future OSD) related members */
	b->texts = NULL;
	b->text_count = 0;

	/* Movement related (& controls) */
	b->moving_left = 0;
	b->moving_right = 0;
	b->lateral_speed = 100;
	b->lateral_tick = 0;

	/* Player related */
	b->score = 0;
	b->paused = 0;
	b->gameover = 0;

	/* Score text is always the first text, then status (pause/gameover). */
	b->score_t = board_add_text(b, (unsigned char *)"0", 260, 172);
	b->status_t = board_add_text(b, (unsigned char *)"", 400, 250);
	b->status_t->effect = EFFECT_SHAKE;
	text_set_color(b->status_t, 100, 20, 30);

	/* Load background. */
	b->bg = loadimage("gfx/gameback.png");

	return b;
}


void
board_kill(Board *board)
{
	int i, size;

	printf("BOARD KILL!!!\n");

	/* Drop clean up */
	for (i = 0; i < board->drop_count; i++) {
		if (board->drops[i] == NULL)
			continue;

		drop_kill(board->drops[i]);
	}
	free(board->drops);
	board->drops = NULL;
	r_free(board->drop_map);
	board->drop_map = NULL;
	board->drop_count = 0;


	/* Output clean up */
	for (i = 0; i < board->output_count; i++) {
		if (board->outputs[i] == NULL)
			continue;

		wateroutput_kill(board->outputs[i]);
	}
	free(board->outputs);
	board->outputs = NULL;
	board->output_count = 0;


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


	/* Block clean up */
	if (board->next_block != NULL) {
		block_kill(board->next_block);
		board->next_block = NULL;
	}

	for (i = 0; i < board->block_count; i++) {
		if (board->blocks[i] == NULL)
			continue;
		block_kill(board->blocks[i]);
	}
	free(board->blocks);
	board->block_count = 0;
	board->blocks = NULL;

	/* text_t cleanup */
	for (i = 0; i < board->text_count; i++) {
		if (board->texts[i] == NULL)
			continue;
		text_kill(board->texts[i]);
	}

	/* General board clean up */
	SDL_FreeSurface(board->bg);
	r_free(board);
}


text_t *
board_add_text(Board *board, unsigned char *value, int x, int y)
{
	text_t *t;

	t = text_new(value);
	
	board->texts = realloc(board->texts, (board->text_count + 1) * sizeof(text_t*));
	board->texts[board->text_count] = t;
	board->text_count++;

	t->x = x;
	t->y = y;

	return t;
}


void
board_refresh_texts(Board *board)
{
	int i;
	text_t *t;
	SDL_Rect r;
	SDL_Surface *s;

	/* Update the score text_t */
	unsigned char score[10];
	snprintf((char *)score, 10, "%d", board->score);
	text_set_value(board->score_t, score);

	/* Draw all the text_ts. */
	for (i = 0; i < board->text_count; i++) {
		t = board->texts[i];
		if (t == NULL)
			continue;

		s = text_get_surface(t);
		text_get_rectangle(t, &r);

		SDL_BlitSurface(s, NULL, screen, &r);
		SDL_FreeSurface(s);
	}
}


void
board_toggle_pause(Board *board)
{
	if (board->paused == 1) {
		board->paused = 0;
		text_set_value(board->status_t, (unsigned char *)"");
	} else {
		board->paused = 1;
		text_set_value(board->status_t, (unsigned char *)"paused!");
	}
}


void
board_refresh(Board *board)
{
	/* Redraw the background. */
	SDL_BlitSurface(board->bg, NULL, screen, NULL);

	/* Redraw each blocks. */
	board_refresh_blocks(board);

	/* Redraw the next block. */
	board_refresh_next(board);

	/* Redraw all the cubes. */
	board_refresh_cubes(board);

	/* Redraw the drops. */
	board_refresh_drops(board);

	/* Draw texts elements (meant to replace OSD) */
	board_refresh_texts(board);

	/* Dig up the back buffer. */
	SDL_Flip(screen);
}


void
board_gameover(Board *board)
{
	board->gameover = 1;
	text_set_value(board->texts[1], (unsigned char *)"game over!");
}


/* board_update() - this function handles all the elements at ticking point */
void
board_update(Board *board, Uint32 now)
{
	if (board->paused == 1 || board->gameover == 1)
		return;

	board_update_blocks(board, now);
	board_update_cubes(board, now);
	board_update_outputs(board, now);
	board_update_drops(board, now);
	board_update_water(board, now);
}

