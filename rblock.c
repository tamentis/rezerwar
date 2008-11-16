#include <stdio.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


extern SDL_Surface *screen;
SDL_Surface *btex = NULL;


void
block_init_btex()
{
	btex = loadimage("gfx/block.png");
}


Block *
block_new(Uint8 size, Uint8 poscount)
{
	Block *block;
	Uint8 i;

	block = r_malloc(sizeof(Block));

	block->size = size;

	block->falling = 1;
	block->tick = 0;

	/* Prepare positions space. */
	block->current_position = 0;
	block->position_count = poscount;
	block->positions = r_malloc(poscount * sizeof(Uint8 *));
	for (i = 0; i < poscount; i++) {
		block->positions[i] = r_malloc(size * size);
	}

	block->cubes = NULL;

	block->x = 0;
	block->y = 0;
	block->prev_y = 0;

	block->type = 0;

	return block;
}


/* block_set_position() - Copy the position configuration to the block. */
void
block_set_position(Block *block, Uint8 pos, Uint8 *p)
{
	Uint8 i;

	for (i = 0; i < block->size * block->size; i++) {
		block->positions[pos][i] = p[i];
	}
	
}


void
block_generate_cubes(Block *block, int n)
{
	int i;

	block->cube_count = n;
	block->cubes = malloc(sizeof(Cube*) * n);

	for (i = 0; i < n; i++) {
		block->cubes[i] = cube_new_random();
	}
}


Block *
block_new_tee()
{
	Block *block;
	Uint8 pos0[] = {  0, 0, 0,
			  1, 2, 3,
			  0, 4, 0 };
	Uint8 pos1[] = {  0, 1, 0,
			  4, 2, 0,
			  0, 3, 0 };
	Uint8 pos2[] = {  0, 4, 0,
			  3, 2, 1,
			  0, 0, 0 };
	Uint8 pos3[] = {  0, 3, 0,
			  0, 2, 4,
			  0, 1, 0 };

	block = block_new(3, 4);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 4);

	block->type = BLOCK_TYPE_TEE;

	return block;
}


Block *
block_new_square()
{
	Block *block;
	Uint8 pos0[] = { 0, 0, 0, 0, 
			 0, 1, 2, 0,
			 0, 4, 3, 0,
			 0, 0, 0, 0 };
	Uint8 pos1[] = { 0, 0, 0, 0, 
			 0, 4, 1, 0,
			 0, 3, 2, 0,
			 0, 0, 0, 0 };
	Uint8 pos2[] = { 0, 0, 0, 0, 
			 0, 3, 4, 0,
			 0, 2, 1, 0,
			 0, 0, 0, 0 };
	Uint8 pos3[] = { 0, 0, 0, 0, 
			 0, 2, 3, 0,
			 0, 1, 4, 0,
			 0, 0, 0, 0 };

	block = block_new(4, 4);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 4);

	block->type = BLOCK_TYPE_SQUARE;

	return block;
}


Block *
block_new_zee()
{
	Block *block;
	Uint8 pos0[] = { 0, 0, 0, 
			 1, 2, 0,
			 0, 3, 4 };
	Uint8 pos1[] = { 0, 0, 1,
			 0, 3, 2,
			 0, 4, 0 };
	Uint8 pos2[] = { 0, 0, 0, 
			 4, 3, 0,
			 0, 2, 1 };
	Uint8 pos3[] = { 0, 0, 4,
			 0, 2, 3,
			 0, 1, 0 };

	block = block_new(3, 4);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 4);

	block->type = BLOCK_TYPE_ZEE;

	return block;
}


Block *
block_new_ess()
{
	Block *block;
	Uint8 pos0[] = { 0, 0, 0,
			 0, 3, 4,
			 1, 2, 0 };
	Uint8 pos1[] = { 0, 1, 0,
			 0, 2, 3,
			 0, 0, 4 };
	Uint8 pos2[] = { 0, 0, 0,
			 0, 2, 1,
			 4, 3, 0 };
	Uint8 pos3[] = { 0, 4, 0,
			 0, 3, 2,
			 0, 0, 1 };

	block = block_new(3, 4);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 4);

	block->type = BLOCK_TYPE_ESS;

	return block;
}


Block *
block_new_bar()
{
	Block *block;
	Uint8 pos0[] = { 0, 0, 0, 0,
			 0, 0, 0, 0,
			 1, 2, 3, 4,
			 0, 0, 0, 0 };
	Uint8 pos1[] = { 0, 1, 0, 0,
			 0, 2, 0, 0,
			 0, 3, 0, 0,
			 0, 4, 0, 0 };
	Uint8 pos2[] = { 0, 0, 0, 0,
			 0, 0, 0, 0,
			 4, 3, 2, 1,
			 0, 0, 0, 0 };
	Uint8 pos3[] = { 0, 4, 0, 0,
			 0, 3, 0, 0,
			 0, 2, 0, 0,
			 0, 1, 0, 0 };

	block = block_new(4, 4);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 4);

	block->type = BLOCK_TYPE_BAR;

	return block;
}


Block *
block_new_ell()
{
	Block *block;
	Uint8 pos0[] = { 0, 0, 0,
			 2, 3, 4,
			 1, 0, 0 };
	Uint8 pos1[] = { 1, 2, 0,
			 0, 3, 0,
			 0, 4, 0 };
	Uint8 pos2[] = { 0, 0, 1,
			 4, 3, 2,
			 0, 0, 0 };
	Uint8 pos3[] = { 0, 4, 0,
			 0, 3, 0,
			 0, 2, 1 };
 
	block = block_new(3, 4);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 4);

	block->type = BLOCK_TYPE_ELL;

	return block;
}


Block *
block_new_jay()
{
	Block *block;
	Uint8 pos0[] = { 0, 0, 0,
			 1, 2, 3,
			 0, 0, 4 };
	Uint8 pos1[] = { 0, 1, 0,
			 0, 2, 0,
			 4, 3, 0 };
	Uint8 pos2[] = { 4, 0, 0,
			 3, 2, 1,
			 0, 0, 0 };
	Uint8 pos3[] = { 0, 3, 4,
			 0, 2, 0,
			 0, 1, 0 };

	block = block_new(3, 4);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block_generate_cubes(block, 4);

	block->type = BLOCK_TYPE_JAY;

	return block;
}


Block *
block_new_random()
{
	long int r = random();
	Block *block;

	switch (r % 7) {
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
		default:
			block = block_new_bar();
			break;
	}

	return block;
}


void
block_kill(Block *block)
{
	Uint8 i;

	for (i = 0; i < block->position_count; i++) {
		r_free(block->positions[i]);
	}
	r_free(block->positions);
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
	Uint8 x, y, i;
	Uint8 *pos = block->positions[block->current_position];
	
	/* All blocks are fixed size. Set DestRect and SourceRect. */
	dr.w = sr.w = BSIZE;
	dr.h = sr.h = BSIZE;
	sr.y = 0;

	s = SDL_CreateRGBSurface(0, block->size * BSIZE, block->size * BSIZE,
		screen->format->BitsPerPixel, 0, 0, 0, 0);

	for (y = 0; y < block->size; y++) {
		for (x = 0; x < block->size; x++) {
			i = y * block->size + x;
			if (pos[i] > 0) {
				cube = block->cubes[pos[i]-1];
				cube_surface = cube_get_surface(cube);

				dr.x = x * BSIZE;
				dr.y = y * BSIZE;

				SDL_BlitSurface(cube_surface, NULL, s, &dr);
			}
		}
	}

	SDL_SetColorKey(s, SDL_SRCCOLORKEY, 0);

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

	if (block->current_position >= block->position_count) {
		block->current_position = 0;
	}

	for (i = 0; i < block->cube_count; i++) {
		cube_rotate_cw(block->cubes[i]);
	}

}

