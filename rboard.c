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


SDL_Surface *
loadimage(char *filename)
{
	SDL_Surface *img;

	img = IMG_Load(filename);

	if (img == NULL) {
		fprintf(stderr, "Unable to load image \"%s\".\n", filename);
		exit(-1);
	}

	return img;
}


void
board_loadbg(Board *b, char *bgfilename)
{
	b->bg = loadimage(bgfilename);
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

	/* Update double-buffering. */
	SDL_Flip(screen);
}

/* board_update() - this function handles all the elements at ticking point */
void
board_update(Board *board, Uint32 now)
{
	board_update_blocks(board, now);
	board_update_cubes(board, now);
	board_update_outputs(board, now);
	board_update_drops(board, now);
	board_update_water(board, now);
}

