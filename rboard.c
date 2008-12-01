#include <stdio.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


extern SDL_Surface *screen;


Board *
board_new(Uint8 width, Uint8 height)
{
	Board *b;
	size_t size = width * height;
	Uint16 i;

	b = r_malloc(sizeof(Board));

	b->width = width;
	b->height = height;

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

	return b;
}


void
board_kill(Board *b)
{
	Uint16 i;

	/* Block clean up */
	if (b->next_block != NULL)
		block_kill(b->next_block);

	for (i = 0; i < b->block_count; i++) {
		if (b->blocks[i] == NULL)
			continue;
		block_kill(b->blocks[i]);
	}
	free(b->blocks);

	/* Drop clean up */
	for (i = 0; i < b->drop_count; i++) {
		if (b->drops[i] == NULL)
			continue;

		drop_kill(b->drops[i]);
	}
	free(b->drops);
	r_free(b->drop_map);


	/* Output clean up */
	for (i = 0; i < b->output_count; i++) {
		if (b->outputs[i] == NULL)
			continue;

		wateroutput_kill(b->outputs[i]);
	}
	free(b->outputs);

	/* General board clean up */
	SDL_FreeSurface(b->bg);
	r_free(b);
}


void
board_loadbg(Board *b, char *bgfilename)
{
	b->bg = loadimage(bgfilename);
}


void
board_refresh_osd(Board *board)
{
	char score[10];

	snprintf(score, 10, "%d", board->score);
	osd_print("rezerwar alpha - press f12 to start", 10, 450);
	osd_print(score, 260, 172);

	if (board->paused == 1)
		osd_print_moving("paused!", 400, 250, 2);
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
	if (board->paused == 1)
		return;

	board_update_blocks(board, now);
	board_update_cubes(board, now);
	board_update_outputs(board, now);
	board_update_drops(board, now);
	board_update_water(board, now);
}

