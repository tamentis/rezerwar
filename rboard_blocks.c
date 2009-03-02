#include <stdio.h>

#include "SDL.h"

#include "rezerwar.h"


extern SDL_Surface *screen;


void
board_add_block(Board *board, Block *block)
{
	board->block_count++;

	board->blocks = realloc(board->blocks,
			board->block_count * sizeof(Block *));

	board->blocks[board->block_count - 1] = block;
}


/**
 * This function will load a different type of block depending on the difficulty of the
 * current board.
 */
void
board_load_next_block(Board *board)
{
	long int r;

	switch (board->difficulty) {
		/* Only one cube blocks */
		case DIFF_EASIEST:
			board->next_block = block_new_one();
			break;
		/* One and two cubes blocks */
		case DIFF_EASY:
			r = rand() % 2 + 7;
			board->next_block = block_new_of_type(r);
			break;
		/* One, two and three cubes blocks */
		case DIFF_MEDIUM:
			r = rand() % 4 + 7;
			board->next_block = block_new_of_type(r);
			break;
		/* All the block types */
		case DIFF_HARD:
		default:
			board->next_block = block_new_random();
			break;
		/* Only the tetrominoes */
		case DIFF_ULTRA:
			r = rand() % 7;
			board->next_block = block_new_of_type(r);
			break;
	}
}


/**
 * Take whatever block is currently loaded as next_block, add it to the
 * list of blocks for this board, place it at the right place and pick a
 * new next_block.
 */
void
board_launch_next_block(Board *board)
{
	Block *nb = board->next_block;

	board_add_block(board, nb);

	nb->x = (byte)(board->width / 2) - (byte)(nb->size / 2);
	nb->y = 0;

	board->current_block = nb;

	board_load_next_block(board);
}



void
board_change_next_block(Board *board)
{
	block_kill(board->next_block);
	board_load_next_block(board);
}


/**
 * Handle the graphic update of the blocks.
 */
void
board_refresh_blocks(Board *board)
{
	byte i;
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


/**
 * Handle the graphic update the of the top right "next" block.
 */
void
board_refresh_next(Board *board)
{
	SDL_Surface *s;
	SDL_Rect r;

	if (board->next_block != NULL) {
		s = block_get_surface(board->next_block);
		block_get_rectangle(board->next_block, &r);

		/* Increment of the position of the preview window. */
		r.x += board->offset_x + 10;
		r.y += 20;

		switch (board->next_block->type) {
			case BLOCK_TYPE_ONE:
				r.x += BSIZE / 2;
				r.y += BSIZE / 2;
				break;
			case BLOCK_TYPE_TWO:
				r.y -= BSIZE / 2;
				break;
			case BLOCK_TYPE_THREE:
				r.x -= BSIZE / 2;
				r.y -= BSIZE / 2;
		}

		SDL_BlitSurface(s, NULL, screen, &r);
		SDL_FreeSurface(s);
	}
}


/**
 * Transfer all the cubes from a block to the board. This is anticipating the
 * death of a block. Set the cube_count to 0 to avoid duplicate killing.
 */
void
board_transfer_cubes(Board *board, Block *block)
{
	int x, y, i, j;
	byte *pos = block->positions[block->current_position];
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

	block->cube_count = 0;
}


/**
 * Loop through all the blocks and call the individual block update, skipping
 * NULL blocks (previously killed). It also handle the speeding up of the game
 * with increasing score.
 */
void
board_update_blocks(Board *board, u_int32_t now)
{
	int i;

	if (board->score < 1000)
		board->block_speed = SPEED_NORMAL;
	else if (board->score < 5000)
		board->block_speed = SPEED_LESS5K;
	else if (board->score < 10000)
		board->block_speed = SPEED_LESS10K;
	else if (board->score < 25000)
		board->block_speed = SPEED_LESS25K;
	else if (board->score < 50000)
		board->block_speed = SPEED_LESS50K;
	else
		board->block_speed = SPEED_MAX;

	if (board->block_speed_factor > 1) {
		board->block_speed /= board->block_speed_factor;
	}

	for (i = 0; i < board->block_count; i++) {
		if (board->blocks[i] == NULL)
			continue;

		board_update_single_block(board, now, i);
	}
}


/**
 * Update an individual block during the current tick (which time is
 * reprensented by 'now'.
 */
void
board_update_single_block(Board *board, u_int32_t now, int i) {
	Block *block = board->blocks[i];

	/* This block's tick has expired, we need to move it. */
	if (block->falling && now - block->tick > board->block_speed) {
		/* Can it fit one unit lower? */
		if (board_move_check(board, block, 0, 1) == 0) {
			block->y++;
			block->tick = now;
		}

		/* If the block didn't move, block it, and un-current */
		if (block->prev_y == block->y) {
			block->falling = 0;
			if (block == board->current_block) {
				board->current_block = NULL;
				if (block->y > 0) {
					sfx_play_tack1();
					board_launch_next_block(board);
				} else {
					board_gameover(board);
				}
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


/* move_check() - 
 * 	bside is the value telling if the cube is on the left of the whole
 * 		block (1) or on the right side (2).
 *
 * Return values:
 * 	0 when path is clear
 * 	3 when touching the bottom
 * 	1 when a cube from the left side blocked
 * 	2 when a cube from the right side blocked */
byte
board_move_check(Board *board, Block *block, Sint8 x, Sint8 y)
{
	byte mx, my;
	byte bside;
	int i, j;
	byte *pos = block->positions[block->current_position];

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
board_set_block_speed(Board *board, u_int32_t speed)
{
	board->block_speed = speed;
}


/**
 * In charge of rotating a block on a board. If there is no space for a
 * rotation, ignore, if against a block, move a bit.
 */
void
board_rotate_cw(Board *board)
{
	Block *block = board->current_block;
	byte x;

	block_rotate_cw(block);

	/* Check if the new position is conflicting, if the conflict is on the
	 * left (1), x+1, if the conflict is on the right, x-1. */
	x = board_move_check(board, block, 0, 0);

	if (x == 1 && board_move_check(board, block, 1, 0) == 0) {
		block->x++;
		return;
	} else if (x == 2 && board_move_check(board, block, -1, 0) == 0) {
		block->x--;
		return;
	}
	
	/* If we had no issue rotating, we're good, play a sound ;) */
	if (x == 0) {
		sfx_play_tick1();
		return;
	}

	/* If we get that far, it's because we didn't manage to rotate the 
	 * block properly... back up. */
	block_rotate_ccw(block);
}


