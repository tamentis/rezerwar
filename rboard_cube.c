/* $Id$ */

#include <stdio.h>

#include "SDL.h"

#include "rezerwar.h"


extern SDL_Surface *screen;


/**
 * Update all the cubes logic (non-graphic stuff).
 */
void
board_update_cubes(Board *board, u_int32_t now)
{
	int i;
	int size = board->width * board->height;
	int type;
	Cube *cube;
	Block *block;

	for (i = 0; i < size; i++) {
		cube = board->cubes[i];

		if (cube == NULL)
			continue;
		
		/* Check if the cube is trashed (if yes fade it until death */
		if (cube->trashed == 1) {
			cube->fade_status++;
			if (cube->fade_status > BSIZE / 2) {
				cube_kill(cube);
				board->cubes[i] = NULL;
			}
			continue;
		}

		/* Check if the cube has free space under itself, if yes 
		 * disconnect it as a cube and create a new block. */
		type = board_get_area_type(board, cube->x, cube->y + 1);
		if (type == ATYPE_FREE) {
			block = block_new_one_from_cube(cube);
			board_add_block(board, block);
			block->x = cube->x;
			block->y = cube->y;
			board->cubes[i] = NULL;
		}
	}
}


/**
 * Refreshing the cubes is actually handling the graphic part, i.e. blitting
 * the textures at the right place.
 */
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
		if (cube == NULL) continue;

		s = cube_get_surface(cube);
		cube_get_rectangle(cube, &r);

		r.x += board->offset_x;
		r.y += board->offset_y;

		SDL_BlitSurface(s, NULL, screen, &r);
		SDL_FreeSurface(s);
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
board_update_water(Board *board, u_int32_t now)
{
	int i;
	Cube *cube;

	board_remove_water(board);

	/* Scan the left side... */
	for (i = 0; i < board->height; i++) {
		cube = board->cubes[i * board->width];
		if (cube == NULL) continue;

		if (cube_plug_match(cube, PLUG_WEST))
			board_spread_water(board, cube, NULL, 1);
	}

	/* Now while scanning the right side, also check if water made it all
	 * the way through, in this case, taint the network. */
	for (i = 0; i < board->height; i++) {
		cube = board->cubes[(i + 1) * board->width - 1];
		if (cube == NULL) continue;

		if (cube_plug_match(cube, PLUG_EAST)) {
			if (cube->water == 1)
				cube_network_taint(cube->root);
			else
				board_spread_water(board, cube, NULL, 2);
		}
	}

	/* Now we know what networks are tainted, go through the left side
	 * again and look for a root cube (net length > 1), with red water (3),
	 * and a network_integrity preserved. Toggle an avalanche if found. */
	for (i = 0; i < board->height; i++) {
		cube = board->cubes[i * board->width];
		if (cube == NULL) continue;

		if (cube->network_size > 1 && cube->water == 3 &&
				cube->network_integrity == 1) {
			board_run_avalanche(board, cube);
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
	int i;
	Text *avtxt;

	/* Start a fading text... */
	avtxt = board_add_text(board, (byte *)"EXCELLENT!", 240, 240);
	text_set_color1(avtxt, 255, 0, 0);
	text_set_color2(avtxt, 80, 0, 0);
	avtxt->effect |= EFFECT_SHAKE|EFFECT_FADEOUT;

	board->score += 200;

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
		if (target != NULL && target->trashed == false) {
			target->trashed = true;
			board->score += 20;
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
		return ATYPE_BLOCK;

	return ATYPE_FREE;
}


/**
 * Given specific x/y offsets, try to spread the water to a neighboring cube
 */
void
board_spread_attempt(Board *board, Cube *cube, Cube *root, Sint8 ox, Sint8 oy,
		byte src_plug, byte dest_plug)
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
				board_spread_water(board, n, root, root->water);
			break;
		default:
			break;
	}
}


/**
 * This function will rotate around the cube and mark as trashed all the neighbors
 * of a cube.
 */
void
board_cube_deflagration(Board *board, Cube *cube)
{
	int i;
	Cube *nc;
	struct _coords {
		int x;
		int y;
	} coords[] = {
		{ -1, -1 }, { 0, -1 }, { 1, -1 },
		{ -1,  0 }, /* cube */ { 1,  0 },
		{ -1,  1 }, { 0,  1 }, { 1,  1 }
	};

	for (i = 0; i < 8; i++) {
		nc = board_get_cube(board, cube->x + coords[i].x, cube->y + coords[i].y);
		if (nc != NULL) {
			nc->trashed = true;
		}
	}
}


/**
 * This function will pick how big a network has to be before we start
 * deflagrating the neighboring cubes.
 */
void
board_network_deflagrate(Board *board, Cube *cube)
{
	int i, max;

	switch (board->difficulty) {
		case DIFF_EASIEST:	max = 5; break;
		case DIFF_EASY:		max = 8; break;
		case DIFF_MEDIUM:	max = 12; break;
		case DIFF_HARD:		max = 16; break;
		case DIFF_ULTRA:	max = 20; break;
	}

	/* Standard circular deflagration */
	if (cube->network_size > max) {
		for (i = 0; i < cube->network_size; i++) {
			board_cube_deflagration(board, cube->network[i]);
		}
		board_cube_deflagration(board, cube);
	}
}


/**
 * A network is going to be destroyed, after a certain size, network explosion is causing
 * deflagration on neighboring blocks.
 */
void
board_destroy_network(Board *board, Cube *cube)
{
	int i;

	if (cube->trashed == 1)
		return;

	printf("board_destroy_network(cube@%dx%d, size=%d)\n", cube->x, cube->y,
			cube->network_size);

	for (i = 0; i < cube->network_size; i++) {
		cube->network[i]->trashed = true;
	}

//	board_network_deflagrate(board, cube);

	cube->trashed = true;
	board->score += (cube->network_size + 1) * 8;

	sfx_play_lazer();
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

	if (root == cube) {
		if (cube->network_integrity == 1) {
			board_destroy_network(board, cube);
		}
	}
}


/**
 * Prepopulate a number of lines at the bottom of the board.
 * (Keywords: generate, line, fill)
 */
void
board_prepopulate(Board *board, int lines)
{
	int x, y;
	Cube *cube;

	for (x = 0; x < board->width; x++) {
		for (y = board->height - lines; y < board->height; y++) {
			cube = cube_new_random();
			cube->x = x;
			cube->y = y;
			board->cubes[y * board->width + x] = cube;
		}
	}
}

