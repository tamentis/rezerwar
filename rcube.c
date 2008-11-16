#include <stdio.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


extern SDL_Surface *screen;
SDL_Surface *ctex = NULL;


/* The following defines the plugs (opened pipe) on the specific cube styles
 * as defined in the block.png file. It is represented as 4 bits as follow:
 *
 *     1
 *   +---+
 * 8 |   | 2
 *   +---+
 *     4
 */

Uint8 cube_plugs[] = {
	0,  9, 11, 10,  8, 15,
	0,  3,  7,  5,  1, 15,
	0,  6, 14, 10,  2, 15,
	0, 12, 13,  5,  4, 15 
};


void
cube_init_texture()
{
	ctex = loadimage("gfx/block.png");
}


/**
 * Return the current 4 bits representing the current opened plugs for the
 * current cube. */
Uint8
cube_get_plugs(Cube *cube)
{
	return cube_plugs[cube->current_position * 6 + cube->type];
}


Cube *
cube_new(Uint8 poscount)
{
	Cube *cube;

	cube = r_malloc(sizeof(Cube));

	/* Prepare positions space. */
	cube->current_position = 0;
	cube->position_count = poscount;
	cube->type = CTYPE_ANGLE;

	cube->x = 0;
	cube->y = 0;

	cube->water = 0;

	cube->network_integrity = 1;
	cube->network_size = 0;
	cube->network = NULL;

	return cube;
}


void
cube_network_add(Cube *root, Cube *cube)
{
	int i = root->network_size++;

	root->network = realloc(root->network, root->network_size * 
			sizeof(Cube *));
	root->network[i] = cube;
}


Cube *
cube_new_random()
{
	Cube *cube;
	int r;
	
	/* Start at a random position. */
	r = random();
	cube = cube_new(r % 3);

	/* Random type. */
	r = random();
	cube->type = r % 5;

	return cube;
}


void
cube_kill(Cube *cube)
{
//	Uint8 i;

	/*
	for (i = 0; i < cube->position_count; i++) {
		r_free(cube->positions[i]);
	}
	r_free(cube->positions);
	*/
	r_free(cube);
}


SDL_Surface *
cube_get_surface(Cube *cube)
{
	SDL_Surface *s;
	SDL_Rect src;
	
	/* All blocks are fixed size. Set DestRect and SourceRect. */
	src.w = BSIZE;
	src.h = BSIZE;
	src.y = 24 + cube->current_position * 16;
	src.x = cube->type * 16;

	s = SDL_CreateRGBSurface(0, BSIZE, BSIZE, screen->format->BitsPerPixel,
			0, 0, 0, 0);

	SDL_BlitSurface(ctex, &src, s, NULL);

	/* If this cube has water, find the water mask 64 px lower. */
	if (cube->water) {
		src.y += 64;
		SDL_BlitSurface(ctex, &src, s, NULL);
	}

	SDL_SetColorKey(s, SDL_SRCCOLORKEY, 0);

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


/**
 * Return true if the cube has the 'mask' plugs opened.
 */
int
cube_plug_match(Cube *cube, Uint8 mask)
{
	Uint8 plugs = cube_get_plugs(cube);

	if ((plugs & mask) == mask)
		return 1;

	return 0;
}


int
cube_get_plug_status(Cube *cube1, Uint8 plug1, Cube *cube2, Uint8 plug2) {
	int status = 0;

	/* Cube1 has an output in this direction */
	if (cube_plug_match(cube1, plug1))
		status += 1;

	/* Cube2 is inexistant. */
	if (cube2 == NULL)
		return status;

	status += 2;

	/* Cube2 has an output in the receptive direciton */
	if (cube_plug_match(cube2, plug2))
		status += 4;

	return status;
}

