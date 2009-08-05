/* $Id$
 *
 * Copyright (c) 2008,2009 Bertrand Janin <tamentis@neopulsar.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"

#include "rezerwar.h"


extern SDL_Surface *screen;
extern SDL_Surface *sprites;
extern Uint32 key;


Block *
block_new(byte size)
{
	Block *block;
	byte i;

	block = r_malloc(sizeof(Block));

	block->size = size;

	block->falling = true;
	block->tick = 0;

	/* Prepare positions space. */
	block->current_position = 0;
	block->positions = r_malloc(4 * sizeof(byte *));
	for (i = 0; i < 4; i++) {
		block->positions[i] = r_malloc(size * size);
	}

	block->cubes = NULL;
	block->existing_cubes = false;

	block->x = 0;
	block->y = 0;
	block->prev_y = 0;

	block->type = 0;

	return block;
}


/* block_set_position() - Copy the position configuration to the block. */
void
block_set_position(Block *block, byte pos, byte *p)
{
	byte i;

	for (i = 0; i < block->size * block->size; i++) {
		block->positions[pos][i] = p[i];
	}
	
}


void
block_generate_cubes(Block *block, int n, bool allow_dynamite)
{
	int i;
	int max = 5;
	unsigned int mask = 0xFFFFFFFF;

	if (allow_dynamite != true) {
		mask ^= 1 << CTYPE_BOMB;
	}

	/* No auto-rocks, no empties */
	mask ^= 1 << CTYPE_EMPTY;
	mask ^= 1 << CTYPE_ROCK;

	block->cube_count = n;
	block->cubes = malloc(sizeof(Cube*) * n);

	for (i = 0; i < n; i++) {
		block->cubes[i] = cube_new_random_mask(mask);
	}
}

Block *
block_new_one_template()
{
	Block *block;
	byte pos0[] = { 1 };

	block = block_new(1);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos0);
	block_set_position(block, 2, pos0);
	block_set_position(block, 3, pos0);

	block->type = BLOCK_TYPE_ONE;

	return block;
}

Block *
block_new_one_from_cube(Cube *cube)
{
	Block *block;

	block = block_new_one_template();
	block->cube_count = 1;
	block->cubes = malloc(sizeof(Cube*));
	block->cubes[0] = cube;

	return block;
}

Block *
block_new_one(bool allow_dynamite)
{
	Block *block;

	block = block_new_one_template();
	block_generate_cubes(block, 1, allow_dynamite);

	return block;
}

Block *
block_new_two()
{
	Block *block;
	byte pos0[] = {  0, 0,
			 1, 2 };
	byte pos1[] = {  1, 0,
			 2, 0 };
	byte pos2[] = {  2, 1,
			 0, 0 };
	byte pos3[] = {  0, 2,
			 0, 1 };

	block = block_new(2);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 2, true);

	block->type = BLOCK_TYPE_TWO;

	return block;
}


Block *
block_new_corner()
{
	Block *block;
	byte pos0[] = {  1, 0,
			 2, 3 };
	byte pos1[] = {  2, 1,
			 3, 0 };
	byte pos2[] = {  3, 2,
			 0, 1 };
	byte pos3[] = {  0, 3,
			 1, 2 };

	block = block_new(2);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 3, true);

	block->type = BLOCK_TYPE_CORNER;

	return block;
}


Block *
block_new_three()
{
	Block *block;
	byte pos0[] = {  0, 0, 0,
			 1, 2, 3,
			 0, 0, 0 };
	byte pos1[] = {  0, 1, 0,
			 0, 2, 0,
			 0, 3, 0 };
	byte pos2[] = {  0, 0, 0,
			 3, 2, 1,
			 0, 0, 0 };
	byte pos3[] = {  0, 3, 0,
			 0, 2, 0,
			 0, 1, 0 };

	block = block_new(3);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 3, true);

	block->type = BLOCK_TYPE_THREE;

	return block;
}


Block *
block_new_tee()
{
	Block *block;
	byte pos0[] = {  0, 0, 0,
			 1, 2, 3,
			 0, 4, 0 };
	byte pos1[] = {  0, 1, 0,
			 4, 2, 0,
			 0, 3, 0 };
	byte pos2[] = {  0, 4, 0,
			 3, 2, 1,
			 0, 0, 0 };
	byte pos3[] = {  0, 3, 0,
			 0, 2, 4,
			 0, 1, 0 };

	block = block_new(3);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 4, true);

	block->type = BLOCK_TYPE_TEE;

	return block;
}


Block *
block_new_square()
{
	Block *block;
	byte pos0[] = { 0, 0, 0, 0, 
			0, 1, 2, 0,
			0, 4, 3, 0,
			0, 0, 0, 0 };
	byte pos1[] = { 0, 0, 0, 0, 
			0, 4, 1, 0,
			0, 3, 2, 0,
			0, 0, 0, 0 };
	byte pos2[] = { 0, 0, 0, 0, 
			0, 3, 4, 0,
			0, 2, 1, 0,
			0, 0, 0, 0 };
	byte pos3[] = { 0, 0, 0, 0, 
			0, 2, 3, 0,
			0, 1, 4, 0,
			0, 0, 0, 0 };

	block = block_new(4);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 4, true);

	block->type = BLOCK_TYPE_SQUARE;

	return block;
}


Block *
block_new_zee()
{
	Block *block;
	byte pos0[] = { 0, 0, 0, 
			1, 2, 0,
			0, 3, 4 };
	byte pos1[] = { 0, 0, 1,
			0, 3, 2,
			0, 4, 0 };
	byte pos2[] = { 0, 0, 0, 
			4, 3, 0,
			0, 2, 1 };
	byte pos3[] = { 0, 0, 4,
			0, 2, 3,
			0, 1, 0 };

	block = block_new(3);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 4, true);

	block->type = BLOCK_TYPE_ZEE;

	return block;
}


Block *
block_new_ess()
{
	Block *block;
	byte pos0[] = { 0, 0, 0,
			0, 3, 4,
			1, 2, 0 };
	byte pos1[] = { 0, 1, 0,
			0, 2, 3,
			0, 0, 4 };
	byte pos2[] = { 0, 0, 0,
			0, 2, 1,
			4, 3, 0 };
	byte pos3[] = { 0, 4, 0,
			0, 3, 2,
			0, 0, 1 };

	block = block_new(3);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 4, true);

	block->type = BLOCK_TYPE_ESS;

	return block;
}


Block *
block_new_bar()
{
	Block *block;
	byte pos0[] = { 0, 0, 0, 0,
			0, 0, 0, 0,
			1, 2, 3, 4,
			0, 0, 0, 0 };
	byte pos1[] = { 0, 1, 0, 0,
			0, 2, 0, 0,
			0, 3, 0, 0,
			0, 4, 0, 0 };
	byte pos2[] = { 0, 0, 0, 0,
			0, 0, 0, 0,
			4, 3, 2, 1,
			0, 0, 0, 0 };
	byte pos3[] = { 0, 4, 0, 0,
			0, 3, 0, 0,
			0, 2, 0, 0,
			0, 1, 0, 0 };

	block = block_new(4);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 4, true);

	block->type = BLOCK_TYPE_BAR;

	return block;
}


Block *
block_new_ell()
{
	Block *block;
	byte pos0[] = { 0, 0, 0,
			2, 3, 4,
			1, 0, 0 };
	byte pos1[] = { 1, 2, 0,
			0, 3, 0,
			0, 4, 0 };
	byte pos2[] = { 0, 0, 1,
			4, 3, 2,
			0, 0, 0 };
	byte pos3[] = { 0, 4, 0,
			0, 3, 0,
			0, 2, 1 };
 
	block = block_new(3);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 4, true);

	block->type = BLOCK_TYPE_ELL;

	return block;
}


Block *
block_new_jay()
{
	Block *block;
	byte pos0[] = { 0, 0, 0,
			1, 2, 3,
			0, 0, 4 };
	byte pos1[] = { 0, 1, 0,
			0, 2, 0,
			4, 3, 0 };
	byte pos2[] = { 4, 0, 0,
			3, 2, 1,
			0, 0, 0 };
	byte pos3[] = { 0, 3, 4,
			0, 2, 0,
			0, 1, 0 };

	block = block_new(3);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 4, true);

	block->type = BLOCK_TYPE_JAY;

	return block;
}


Block *
block_new_of_type(int type)
{
	Block *block;

	switch (type) {
		case BLOCK_TYPE_TEE:
			block = block_new_tee();
			break;
		case BLOCK_TYPE_ELL:
			block = block_new_ell();
			break;
		case BLOCK_TYPE_JAY:
			block = block_new_jay();
			break;
		case BLOCK_TYPE_ZEE:
			block = block_new_zee();
			break;
		case BLOCK_TYPE_ESS:
			block = block_new_ess();
			break;
		case BLOCK_TYPE_SQUARE:
			block = block_new_square();
			break;
		case BLOCK_TYPE_ONE:
			block = block_new_one(true);
			break;
		case BLOCK_TYPE_TWO:
			block = block_new_two();
			break;
		case BLOCK_TYPE_THREE:
			block = block_new_three();
			break;
		case BLOCK_TYPE_CORNER:
			block = block_new_corner();
			break;
		default: /* 0 */
			block = block_new_bar();
			break;
	}

	return block;
}


Block *
block_new_random()
{
	long int r = rand();
	Block *block;

	block = block_new_of_type(r % 11);

	return block;
}


void
block_kill(Block *block)
{
	byte i;

	for (i = 0; i < 4; i++) {
		r_free(block->positions[i]);
	}
	r_free(block->positions);
	block->positions = NULL;

	for (i = 0; i < block->cube_count; i++) {
		cube_kill(block->cubes[i]);
	}
	free(block->cubes);
	block->cubes = NULL;

	r_free(block);
}


SDL_Surface *
block_get_surface(Block *block)
{
	SDL_Surface *s;
	SDL_Surface *cube_surface;
	Cube *cube;
	SDL_Rect sr;
	SDL_Rect dr;
	byte x, y, i;
	byte *pos = block->positions[block->current_position];
	
	/* All blocks are fixed size. Set DestRect and SourceRect. */
	dr.w = sr.w = BSIZE;
	dr.h = sr.h = BSIZE;
	sr.y = 0;

	s = SDL_CreateRGBSurface(0, block->size * BSIZE, block->size * BSIZE,
		screen->format->BitsPerPixel, 0, 0, 0, 0);
	SDL_FillRect(s, NULL, key);
	SDL_SetColorKey(s, SDL_SRCCOLORKEY|SDL_RLEACCEL, key);


	for (y = 0; y < block->size; y++) {
		for (x = 0; x < block->size; x++) {
			i = y * block->size + x;
			if (pos[i] > 0) {
				cube = block->cubes[pos[i]-1];
				cube_surface = cube_get_surface(cube);

				dr.x = x * BSIZE;
				dr.y = y * BSIZE;

				SDL_BlitSurface(cube_surface, NULL, s, &dr);
				SDL_FreeSurface(cube_surface);
			}
		}
	}

	return s;
}


void
block_get_rectangle(Block *block, SDL_Rect *r)
{
	r->w = block->size * BSIZE;
	r->h = block->size * BSIZE;
	r->x = block->x * BSIZE;
	r->y = block->y * BSIZE;
}


void
block_rotate_cw(Block *block)
{
	int i;

	block->current_position++;

	if (block->current_position >= 4)
		block->current_position = 0;

	for (i = 0; i < block->cube_count; i++) {
		cube_rotate_cw(block->cubes[i]);
	}

}


void
block_rotate_ccw(Block *block)
{
	int i;

	block->current_position--;

	if (block->current_position < 0)
		block->current_position = 3;

	for (i = 0; i < block->cube_count; i++) {
		cube_rotate_ccw(block->cubes[i]);
	}

}

