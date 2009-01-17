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

	/* Movement related (& controls) */
	b->moving_left = 0;
	b->moving_right = 0;
	b->lateral_speed = 100;
	b->lateral_tick = 0;

	/* Player related */
	b->score = 0;
	b->paused = 0;
	b->gameover = 0;

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


	/* General board clean up */
	SDL_FreeSurface(board->bg);
	r_free(board);
}


void
board_refresh_osd(Board *board)
{
	char score[10];

	snprintf(score, 10, "%d", board->score);
	osd_print("rezerwar alpha 2008-12-28 / press f12 to start", 10, 450);
	osd_print(score, 260, 172);

	if (board->paused == 1)
		osd_print_moving("paused!", 400, 250, 2);

	if (board->gameover == 1)
		osd_print_moving("game over!", 250, 240, 2);
}


void
board_toggle_pause(Board *board)
{
	board->paused = board->paused == 1 ? 0 : 1;
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

	/* Draw score */
	board_refresh_osd(board);

	/* Update double-buffering. */
	SDL_Flip(screen);
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

