#include <stdio.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


extern SDL_Surface *screen;


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
 * Take a cube and check its neighbors for propagation. 'n' is always the
 * neighbor cube.
 */
void
board_spread_water(Board *board, Cube *cube, Cube *root)
{
	Cube *n;
	int status;

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

	/* North cube */
	n = board_get_cube(board, cube->x, cube->y - 1);
	status = cube_get_plug_status(cube, PLUG_NORTH, n, PLUG_SOUTH);
	if (status == PSTAT_CONNECTED) {
		board_spread_water(board, n, cube);
	} else if (status == PSTAT_OPENED && root != NULL) {
		root->network_integrity = 0;
	}

	/* East cube */
	n = board_get_cube(board, cube->x + 1, cube->y);
	status = cube_get_plug_status(cube, PLUG_EAST, n, PLUG_WEST);
	if (status == PSTAT_CONNECTED) {
		board_spread_water(board, n, cube);
	} else if (status == PSTAT_OPENED && root != NULL) {
		root->network_integrity = 0;
	}

	/* South cube */
	n = board_get_cube(board, cube->x, cube->y + 1);
	status = cube_get_plug_status(cube, PLUG_SOUTH, n, PLUG_NORTH);
	if (status == PSTAT_CONNECTED) {
		board_spread_water(board, n, cube);
	} else if (status == PSTAT_OPENED && root != NULL) {
		root->network_integrity = 0;
	}

	/* West cube */
	n = board_get_cube(board, cube->x - 1, cube->y);
	status = cube_get_plug_status(cube, PLUG_WEST, n, PLUG_EAST);
	if (status == PSTAT_CONNECTED) {
		board_spread_water(board, n, cube);
	} else if (status == PSTAT_OPENED && root != NULL) {
		root->network_integrity = 0;
	}

	if (root == cube) {
		printf("INTEGRITY:%d\n", cube->network_integrity);
	}
}

