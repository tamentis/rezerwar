#include <stdio.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


extern SDL_Surface *screen;


void
board_add_block(Board *b, Block *block)
{
	b->block_count++;

	b->blocks = realloc(b->blocks, b->block_count * sizeof(Block *));

	b->blocks[b->block_count - 1] = block;
}


void
board_load_next_block(Board *board)
{
	Block *block;

	block = block_new_random();

	board->next_block = block;
}


/* board_launch_next_block() - Take whatever block is currently loaded as
 * next_block, add it to the list of blocks for this board, place it at 
 * the right place and pick a new next_block. */
void
board_launch_next_block(Board *board)
{
	Block *nb = board->next_block;

	board_add_block(board, nb);

	nb->x = (Uint8)(board->width / 2) - (Uint8)(nb->size / 2);
	nb->y = 0;

	board->current_block = nb;

	board_load_next_block(board);
}


void
board_refresh_blocks(Board *board)
{
	Uint8 i;
	SDL_Rect r;
	Block *block;
	SDL_Surface *s;

	for (i = 0; i < board->block_count; i++) {
		block = board->blocks[i];
		if (block == NULL)
			continue;

		s = block_get_surface(block);
	//	fprintf(stderr, "refreshing block %d %p\n", i, s);
		block_get_rectangle(block, &r);

		r.x += board->offset_x;
		r.y += board->offset_y;

		SDL_BlitSurface(s, NULL, screen, &r);

		SDL_FreeSurface(s);
	}
}


void
board_refresh_next(Board *board)
{
	SDL_Surface *s;
	SDL_Rect r;

	if (board->next_block != NULL) {
		s = block_get_surface(board->next_block);
		block_get_rectangle(board->next_block, &r);

		/* Increment of the position of the preview window. */
		r.x += board->offset_x + board->width * BSIZE + BSIZE;
		r.y += board->offset_y + BSIZE;

		/* Increment of the alignement correction. */
		r.x += (Uint8)((5 - board->next_block->size) * BSIZE / 2);
		r.y += (Uint8)((5 - board->next_block->size) * BSIZE / 2);

		/* Squares get a special favor. */
		if (board->next_block->type == BLOCK_TYPE_SQUARE)
			r.y += (Uint8)(BSIZE / 2);

		SDL_BlitSurface(s, NULL, screen, &r);

		SDL_FreeSurface(s);
	}
}


/**
 * Transfer all the cubes from a block to the board. This is anticipating the
 * death of a block. */
void
board_transfer_cubes(Board *board, Block *block)
{
	int x, y, i, j;
	Uint8 *pos = block->positions[block->current_position];
	Cube *cube;

	for (y = 0; y < block->size; y++) {
		for (x = 0; x < block->size; x++) {
			i = (y + block->y) * board->width + (x + block->x);
			j = y * block->size + x;

			if (pos[j] < 1)
				continue;

			cube = block->cubes[pos[j] - 1];

			board->cubes[i] = cube;
				
			if (cube != NULL) {
				cube->x = block->x + x;
				cube->y = block->y + y;
			}

			block->cubes[pos[j] - 1] = NULL;
		}
	}
}


void
board_update_blocks(Board *board, Uint32 now)
{
	Uint16 i;
	Block *block;

	/* How often to update blocks. */
	for (i = 0; i < board->block_count; i++) {
		block = board->blocks[i];
		if (block == NULL)
			continue;

		if (block->falling && now - block->tick > board->block_speed) {
			/* Can this block fit one unit lower? */
			if (board_move_check(board, block, 0, 1) == 0) {
				block->y++;
				block->tick = now;
			}

			/* If the block didn't move, block it, and un-current */
			if (block->prev_y == block->y) {
				board->current_block = NULL;
				block->falling = 0;
				if (block->y > 0) {
					printf("launch_next_block()\n");
					board_launch_next_block(board);
				} else {
					printf("STOPPING (too high)\n");
				}
				board_transfer_cubes(board, block);
				block_kill(block);
				board->blocks[i] = NULL;
				return;

			}

			/* Keep a reference to see if the block moved. */
			block->prev_y = block->y;
		}

		/* Ticking for the lateral moves. */
		if (now - board->lateral_tick > board->lateral_speed) {
			if (board->moving_left > 1) {
				board->moving_left--;
			} else if (board->moving_left == 1) {
				board_move_current_block_left(board);
			}

			if (board->moving_right > 1) {
				board->moving_right--;
			} else if (board->moving_right == 1) {
				board_move_current_block_right(board);
			}

			board->lateral_tick = now;
		}
	}
}


#if 0
void
board_dump_block_map(Board *board)
{
	Uint8 x, y;
	Uint16 i;

	for (y = 0; y < board->height; y++) {
		for (x = 0; x < board->width; x++) {
			i = y * board->width + x;
			if (board->map[i]) {
				printf("x");
			} else {
				printf(" ");
			}
		}
		printf("\n");
	}

}


void
board_add_block_to_map(Board *board, Block* block)
{
	Uint8 x, y;
	Uint16 i, j;
	Uint8 *pos = block->positions[block->current_position];

	for (y = 0; y < block->size; y++) {
		for (x = 0; x < block->size; x++) {
			j = y * block->size + x;
			i = (block->y + y) * board->width + (block->x + x);
			if (pos[j]) {
				board->map[i] = 1;
			}
		}
	}
}





/* board_update_map() - Loop through all the blocks and populate the board
 * map with inactive blocks. */
void
board_update_map(Board *board)
{
	Uint8 i;

	memset(board->map, 0, board->width * board->height);
	for (i = 0; i < board->block_count; i++) {
		if (board->blocks[i] == NULL)
			continue;

		/* Don't count the current block. */
		if (board->blocks[i] == board->current_block)
			continue;

		board_add_block_to_map(board, board->blocks[i]);
	}
}
#endif


/* move_check() - 
 * 	bside is the value telling if the cube is on the left of the whole
 * 		block (1) or on the right side (2).
 *
 * Return values:
 * 	0 when path is clear
 * 	3 when touching the bottom
 * 	1 when a cube from the left side blocked
 * 	2 when a cube from the right side blocked */
Uint8
board_move_check(Board *board, Block *block, Sint8 x, Sint8 y)
{
	Uint8 mx, my;
	Uint8 bside;
	Uint16 i, j;
	Uint8 *pos = block->positions[block->current_position];

	/* Update the map. */
//	board_update_map(board);

	/* For every cubes in this block, you need to check if there is a
	 * space in whatever direction. */
	for (my = 0; my < block->size; my++) {
		for (mx = 0; mx < block->size; mx++) {
			i = my * block->size + mx;
			bside = mx < block->size / 2 ? 1 : 2;

			/* Skip void cubes. */
			if (pos[i] == 0)
				continue;

			/* Reached the bottom of the board. */
			if (block->y + my + y >= board->height)
				return 3;

			/* Reached the left border. */
			if (block->x + mx + x < 0)
				return 1;

			/* Reach the right border. */
			if (block->x + mx + x >= board->width)
				return 2;
			
			/* There is a block in this direction. */
			j = (my + block->y + y) * board->width + (mx + block->x + x);
			if (board->cubes[j]) {
				return bside;
			}
		}
	}

	return 0;
}


void
board_move_current_block_left(Board *board)
{
	Block *block = board->current_block;

	if (block == NULL)
		return;

	if (board_move_check(board, block, -1, 0) == 0) {
		block->x--;
	}
//	board_dump_block_map(board);
}


void
board_move_current_block_right(Board *board)
{
	Block *block = board->current_block;

	if (block == NULL)
		return;

	if (board_move_check(board, block, 1, 0) == 0) {
		block->x++;
	}
}


void
board_set_block_speed(Board *board, Uint32 speed)
{
	board->block_speed = speed;
}


void
board_rotate_cw(Board *board)
{
	Block *block = board->current_block;
	Uint8 x;

	block_rotate_cw(block);

	/* Check if the new position is conflicting, if the conflict is on the
	 * left (1), x+1, if the conflict is on the right, x-1. */
	x = board_move_check(board, block, 0, 0);
	if (x == 1)
		block->x++;
	else if (x == 2)
		block->x--;
}



