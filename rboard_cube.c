/* $Id$ */

#include <stdio.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


extern SDL_Surface *screen;


void
board_update_cubes(Board *board, Uint32 now)
{
	int i;
	int size = board->width * board->height;
	Cube *cube;

	for (i = 0; i < size; i++) {
		cube = board->cubes[i];
		
		if (cube != NULL && cube->trashed == 1) {
			cube_kill(cube);
			board->cubes[i] = NULL;
		}
	}
}


void
board_refresh_cubes(Board *board)
{
	int i;
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
		board->cubes[i]->network_integrity = 1;
		cube_network_flush(board->cubes[i]);
	}
}


void
board_update_water(Board *board, Uint32 now)
{
	int i;
	Cube *cube;

	board_remove_water(board);

	/* Check the whole left side for cubes. If any, run the routine for 
	 * path finder. */
	for (i = 0; i < board->height; i++) {
		cube = board->cubes[i * board->width];
		if (cube == NULL)
			continue;

		if (cube_plug_match(cube, PLUG_WEST)) {
			board_spread_water(board, cube, NULL);
		}

	}
}


/**
 * Return a cube at the given coordinates, return NULL if not found or if
 * border of the board.
 */
Cube *
board_get_cube(Board *board, Sint16 x, Sint16 y)
{
	/* Bad x value */
	if (x < 0 || x > board->width)
		return NULL;

	/* Bad y value */
	if (y < 0 || y > board->height)
		return NULL;

	return (board->cubes[x + board->width * y]);
}


/**
 * Returns an integer representing the type of area at x,y
 */
int
board_get_area_type(Board *board, Sint16 x, Sint16 y)
{
	if (x < 0)
		return ATYPE_BOARD_LEFT;

	if (x > board->width)
		return ATYPE_BOARD_RIGHT;

	if (y >= board->height)
		return ATYPE_BOARD_BOTTOM;

	if (board->cubes[x + board->width * y] != NULL)
		return ATYPE_BLOCK;

	return ATYPE_FREE;
}


void
board_spread_attempt(Board *board, Cube *cube, Cube *root, Sint8 ox, Sint8 oy,
		Uint8 src_plug, Uint8 dest_plug)
{
	Cube *n;
	int status;
	int type;

	type = board_get_area_type(board, cube->x + ox, cube->y + oy);
	switch (type) {
		case ATYPE_FREE:
			if (cube_plug_match(cube, src_plug))
				root->network_integrity = 0;
			break;
		case ATYPE_BLOCK:
			n = board_get_cube(board, cube->x + ox, cube->y + oy);
			status = cube_get_plug_status(cube, src_plug, n, 
					dest_plug);
			if (status == PSTAT_CONNECTED)
				board_spread_water(board, n, root);
			break;
		default:
			break;
	}
}


void
board_destroy_network(Board *board, Cube *cube)
{
	int i;

	printf("board_destroy_network(cube@%dx%d, size=%d)\n", cube->x, cube->y,
			cube->network_size);

	for (i = 0; i < cube->network_size; i++) {
		cube->network[i]->trashed = 1;
	}

	cube->trashed = 1;
	board->score += (cube->network_size + 1) * 8;
}


/**
 * Take a cube and check its neighbors for propagation. 'n' is always the
 * neighbor cube.
 */
void
board_spread_water(Board *board, Cube *cube, Cube *root)
{
	/* Don't continue if this cube is already watered. */
	if (cube->water == 1)
		return;

	/* Add 'cube' to the network, it doesn't seem to be a starting point. */
	if (root == NULL) {
		root = cube;
	} else {
		cube_network_add(root, cube);
	}

	cube->water = 1;

	/* North, East, South, West */
//	printf("cube@%dx%d: spread_attempt NORTH\n", cube->x, cube->y);
	board_spread_attempt(board, cube, root,  0, -1, PLUG_NORTH, PLUG_SOUTH);
//	printf("cube@%dx%d: spread_attempt EAST\n", cube->x, cube->y);
	board_spread_attempt(board, cube, root,  1,  0, PLUG_EAST,  PLUG_WEST);
//	printf("cube@%dx%d: spread_attempt SOUTH\n", cube->x, cube->y);
	board_spread_attempt(board, cube, root,  0,  1, PLUG_SOUTH, PLUG_NORTH);
//	printf("cube@%dx%d: spread_attempt WEST\n", cube->x, cube->y);
	board_spread_attempt(board, cube, root, -1,  0, PLUG_WEST,  PLUG_EAST);

	if (root == cube) {
		if (cube->network_integrity == 1) {
			board_destroy_network(board, cube);
		}
	}
}

