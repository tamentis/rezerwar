#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"

#include "rezerwar.h"


extern SDL_Surface *screen;
extern SDL_Surface *sprites;
extern Uint32 key;


/* The following defines the plugs (opened pipe) on the specific cube styles
 * as defined in the block.png file. It is represented as 4 bits as follow:
 *
 *     1
 *   +---+
 * 8 |   | 2
 *   +---+
 *     4
 */

byte cube_plugs[] = {
	0,  9, 11, 10,  8, 15, 0,
	0,  3,  7,  5,  1, 15, 0,
	0,  6, 14, 10,  2, 15, 0,
	0, 12, 13,  5,  4, 15, 0
};



/**
 * Return the current 4 bits representing the current opened plugs for the
 * current cube. */
byte
cube_get_plugs(Cube *cube)
{
	return cube_plugs[cube->current_position * 7 + cube->type];
}


Cube *
cube_new(byte start_pos)
{
	Cube *cube;

	cube = r_malloc(sizeof(Cube));

	/* Prepare positions space. */
	cube->current_position = start_pos;
	cube->type = CTYPE_ANGLE;

	cube->x = 0;
	cube->y = 0;

	cube->water = 0;

	cube->network_integrity = 1;
	cube->network_size = 0;
	cube->network = NULL;
	cube->root = NULL;

	cube->fade_status = 0;

	cube->trashed = 0;

	return cube;
}

/**
 * Shortcut to assign the type directly.
 */
Cube *
cube_new_type(byte start_pos, int type)
{
	Cube *cube;

	cube = cube_new(start_pos);
	cube->type = type;

	return cube;
}

/**
 * Create a rock, random color.
 */
Cube *
cube_new_rock()
{
	Cube *cube;

	cube = cube_new(rand() % 4);
	cube->type = CTYPE_ROCK;

	return cube;
}

void
cube_network_add(Cube *root, Cube *cube)
{
	int i = root->network_size++;

	root->network = realloc(root->network, root->network_size * 
			sizeof(Cube *));
	root->network[i] = cube;
	cube->root = root;
}


/**
 * Remove all the connections in this cube's network.
 */
void
cube_network_flush(Cube *cube)
{
	cube->network_size = 0;
	free(cube->network);
	cube->network = NULL;
}


/**
 * Taint the network linked to this cube.
 */
void
cube_network_taint(Cube *cube)
{
	int i;

	cube->water = 3;
	for (i = 0; i < cube->network_size; i++) {
		cube->network[i]->water = 3;
	}
}

/**
 * Core function for generating cubes. It takes an argument to know the
 * maximum type to allow in the pick.
 */
Cube *
cube_new_random_max(int max)
{
	Cube *cube;
	int r;
	
	/* Start at a random position. */
	r = rand();
	cube = cube_new(r % 4);

	/* Random type. (skipping the first blank) */
	r = rand();
	cube->type = 1 + r % max;

	return cube;
}

/**
 * Wrapper around the core generator which only returns the 5 first types,
 * this is the default as it is used to generate the original flooring.
 */
Cube *
cube_new_random()
{
	Cube *cube;

	cube = cube_new_random_max(5);

	return cube;
}


/**
 * Given one char representing a character, instanciate a new cube.
 */
Cube *
cube_new_from_char(char c)
{
	Cube *cube = NULL;

	switch (c) {
		case '+': cube = cube_new_type(0, CTYPE_ALL); break;
		case '-': cube = cube_new_type(0, CTYPE_FLAT); break;
		case '|': cube = cube_new_type(1, CTYPE_FLAT); break;
		case 'J': cube = cube_new_type(0, CTYPE_ANGLE); break;
		case 'L': cube = cube_new_type(1, CTYPE_ANGLE); break;
		case 'F': cube = cube_new_type(2, CTYPE_ANGLE); break;
		case '7': cube = cube_new_type(3, CTYPE_ANGLE); break;
		case '>': cube = cube_new_type(0, CTYPE_KNOB); break;
		case '_': cube = cube_new_type(1, CTYPE_KNOB); break;
		case '<': cube = cube_new_type(2, CTYPE_KNOB); break;
		case '^': cube = cube_new_type(3, CTYPE_KNOB); break;
		case 'A': cube = cube_new_type(0, CTYPE_TEE); break;
		case '}': cube = cube_new_type(1, CTYPE_TEE); break;
		case 'T': cube = cube_new_type(2, CTYPE_TEE); break;
		case '{': cube = cube_new_type(3, CTYPE_TEE); break;
		case 'R': cube = cube_new_rock(); break;
		case '.':
		default:
			break;
	}

	return cube;
}


void
cube_kill(Cube *cube)
{
	cube_network_flush(cube);
	r_free(cube);
}


SDL_Surface *
cube_get_surface(Cube *cube)
{
	SDL_Surface *s;
	SDL_Rect src;
	SDL_Rect *dst = NULL;
	int fs = cube->fade_status;
	
	/* All blocks are fixed size. Set DestRect and SourceRect. */
	src.w = BSIZE;
	src.h = BSIZE;
	src.y = cube->current_position * BSIZE;
	src.x = (cube->type - 1) * BSIZE;

	s = SDL_CreateRGBSurface(0, BSIZE, BSIZE, screen->format->BitsPerPixel,
			0, 0, 0, 0);
	SDL_SetColorKey(s, SDL_SRCCOLORKEY|SDL_RLEACCEL, key);
	SDL_FillRect(s, NULL, key);

	/* If we have a fade_status, we need to crop a smaller area. */
	if (fs > 0) {
		dst = r_malloc(sizeof(SDL_Rect));
		dst->x = dst->y = fs;
		dst->w = dst->h = fs * 2;
		src.x += fs;
		src.y += fs;
		src.w -= fs * 2;
		src.h -= fs * 2;
	}

	SDL_BlitSurface(sprites, &src, s, dst);

	/* If this cube has water, find the water mask 64px lower or 128px
	 * if this is type 2 water (from the right side). */
	if (cube->water) {
		src.y += BSIZE * 4 * cube->water;
		SDL_BlitSurface(sprites, &src, s, dst);
	}
	
	r_free(dst);

	return s;
}


void
cube_get_rectangle(Cube *cube, SDL_Rect *r)
{
	r->w = BSIZE;
	r->h = BSIZE;
	r->x = cube->x * BSIZE;
	r->y = cube->y * BSIZE;
}


void
cube_rotate_cw(Cube *cube)
{
	cube->current_position++;

	if (cube->current_position >= 4) {
		cube->current_position = 0;
	}
}


void
cube_rotate_ccw(Cube *cube)
{
	cube->current_position--;

	if (cube->current_position < 0) {
		cube->current_position = 3;
	}
}


/**
 * Return true if the cube has the 'mask' plugs opened.
 */
int
cube_plug_match(Cube *cube, byte mask)
{
	byte plugs = cube_get_plugs(cube);

	if ((plugs & mask) == mask)
		return 1;

	return 0;
}


int
cube_get_plug_status(Cube *cube1, byte plug1, Cube *cube2, byte plug2) {
	int status = 0;

	/* Cube1 has an output in this direction */
	if (cube_plug_match(cube1, plug1))
		status += 1;

	/* Cube2 is inexistant. */
	if (cube2 == NULL)
		return status;

	status += 2;

	/* Cube2 has an output in the receptive direction */
	if (cube_plug_match(cube2, plug2))
		status += 4;

	return status;
}


