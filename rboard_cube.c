#include <stdio.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


extern SDL_Surface *screen;


/**
 * This is a test function used for debugging, in real context, you would
 * not spawn a cube by itself. */
void
board_add_cube(Board *b)
{
	Cube *cube;
	
	fprintf(stderr, "board_add_cube()\n");

	cube = cube_new(1);
	cube->x = (Uint8)(b->width / 2) - 1;
	cube->y = 0;

	b->cube_count++;

	b->cubes = realloc(b->cubes, b->cube_count * sizeof(Cube *));

	b->cubes[b->cube_count - 1] = cube;
}


void
board_refresh_cubes(Board *board)
{
	Uint8 i;
	SDL_Rect r;
	Cube *cube;
	SDL_Surface *s;
	int size = board->width * board->height;

	for (i = 0; i < size; i++) {
		cube = board->cubes[i];
		if (cube == NULL)
			continue;

		s = cube_get_surface(cube);
//		fprintf(stderr, "refreshing cube %d %p\n", i, s);
		cube_get_rectangle(cube, &r);

		r.x += board->offset_x;
		r.y += board->offset_y;

		SDL_BlitSurface(s, NULL, screen, &r);

		SDL_FreeSurface(s);
	}
}


void
board_water_all_cubes(Board *board)
{
	int x, y, i;

	fprintf(stderr, "water_all_cubes\n");

	for (y = 0; y < board->height; y++) {
		for (x = 0; x < board->width; x++) {
			i = y * board->width + x;

			if (board->cubes[i] == NULL)
				continue;

			board->cubes[i]->water = 1;
		}
	}
}


void
board_dump_cube_map(Board *board)
{
	Uint8 x, y;
	Uint16 i;

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


#if 0
void
void
board_update_cubes(Board *board, Uint32 now)
{
	Uint16 i;
	cube *cube;

	/* How often to update cubes. */
	for (i = 0; i < board->cube_count; i++) {
		cube = board->cubes[i];
		if (cube == NULL)
			continue;

		if (cube->falling && now - cube->tick > board->cube_speed) {
			/* Can this cube fit one unit lower? */
			if (board_move_check(board, cube, 0, 1) == 0) {
				cube->y++;
				cube->tick = now;
			}

			/* If the cube didn't move, cube it, and un-current */
			if (cube->prev_y == cube->y) {
				board->current_cube = NULL;
				cube->falling = 0;
				/* XXX: remove that and implement properly. */
				if (cube->y > 0) {
//					board_launch_next_cube(board);
					printf("launch_next_cube()\n");
				} else {
					printf("STOPPING (too high)\n");
				}

			}

			/* Keep a reference to see if the cube moved. */
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
}


void
board_add_cube_to_map(Board *board, cube* cube)
{
	Uint8 x, y;
	Uint16 i, j;
	Uint8 *pos = cube->positions[cube->current_position];

	for (y = 0; y < cube->size; y++) {
		for (x = 0; x < cube->size; x++) {
			j = y * cube->size + x;
			i = (cube->y + y) * board->width + (cube->x + x);
			if (pos[j]) {
				board->map[i] = 1;
			}
		}
	}
}





/* board_update_map() - Loop through all the cubes and populate the board
 * map with inactive cubes. */
void
board_update_map(Board *board)
{
	Uint8 i;

	memset(board->map, 0, board->width * board->height);
	for (i = 0; i < board->cube_count; i++) {
		if (board->cubes[i] == NULL)
			continue;

		/* Don't count the current cube. */
		if (board->cubes[i] == board->current_cube)
			continue;

		board_add_cube_to_map(board, board->cubes[i]);
	}
}


/* move_check() - 
 * 	bside is the value telling if the cube is on the left of the whole
 * 		cube (1) or on the right side (2).
 *
 * Return values:
 * 	0 when path is clear
 * 	3 when touching the bottom
 * 	1 when a cube from the left side cubeed
 * 	2 when a cube from the right side cubeed */
Uint8
board_move_check(Board *board, cube *cube, Sint8 x, Sint8 y)
{
	Uint8 mx, my;
	Uint8 bside;
	Uint16 i, j;
	Uint8 *pos = cube->positions[cube->current_position];

	/* Update the map. */
	board_update_map(board);

	/* For every cubes in this cube, you need to check if there is a
	 * space in whatever direction. */
	for (my = 0; my < cube->size; my++) {
		for (mx = 0; mx < cube->size; mx++) {
			i = my * cube->size + mx;
			bside = mx < cube->size / 2 ? 1 : 2;

			/* Skip void cubes. */
			if (pos[i] == 0)
				continue;

			/* Reached the bottom of the board. */
			if (cube->y + my + y >= board->height)
				return 3;

			/* Reached the left border. */
			if (cube->x + mx + x < 0)
				return 1;

			/* Reach the right border. */
			if (cube->x + mx + x >= board->width)
				return 2;
			
			/* There is a cube in this direction. */
			j = (my + cube->y + y) * board->width + (mx + cube->x + x);
			if (board->map[j]) {
				return bside;
			}
		}
	}

	return 0;
}


void
board_move_current_cube_left(Board *board)
{
	cube *cube = board->current_cube;

	if (cube == NULL)
		return;

	if (board_move_check(board, cube, -1, 0) == 0) {
		cube->x--;
	}
//	board_dump_cube_map(board);
}


void
board_move_current_cube_right(Board *board)
{
	cube *cube = board->current_cube;

	if (cube == NULL)
		return;

	if (board_move_check(board, cube, 1, 0) == 0) {
		cube->x++;
	}
}


void
board_set_cube_speed(Board *board, Uint32 speed)
{
	board->cube_speed = speed;
}


void
board_rotate_cw(Board *board)
{
	cube *cube = board->current_cube;
	Uint8 x;

	cube->current_position++;

	if (cube->current_position >= cube->position_count) {
		cube->current_position = 0;
	}

	/* Check if the new position is conflicting, if the conflict is on the
	 * left (1), x+1, if the conflict is on the right, x-1. */
	x = board_move_check(board, cube, 0, 0);
	if (x == 1)
		cube->x++;
	else if (x == 2)
		cube->x--;
}
#endif
