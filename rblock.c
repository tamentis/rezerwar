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

/*
SDL_Surface *
get_baseblock()
{
	SDL_Surface *x;
	SDL_Rect r;

	if (baseblock_r == NULL) {
		x = loadimage("gfx/block.png");
		baseblock_r = SDL_CreateRGBSurface(SDL_SWSURFACE, BSIZE, BSIZE,
				x->format->BitsPerPixel, 0, 0, 0, 0);

		r.w = r.h = BSIZE;
		r.x = BSIZE;
		r.y = 0;
		SDL_BlitSurface(x, &r, baseblock_r, NULL);

		SDL_FreeSurface(x);
	}

	return baseblock_r;
}
*/


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


Block *
block_new_tee()
{
	Block *block;
	Uint8 pos0[] = {  0,  0,  0,
			  9, 12, 10,
			  0,  8,  0 };
	Uint8 pos1[] = {  0,  7,  0,
			  9, 13,  0,
			  0,  8,  0 };
	Uint8 pos2[] = {  0,  7,  0,
			  9, 11, 10,
			  0,  0,  0 };
	Uint8 pos3[] = {  0,  7,  0,
			  0, 14, 10,
			  0,  8,  0 };

	block = block_new(3, 4);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block->type = BLOCK_TYPE_TEE;

	return block;
}


Block *
block_new_square()
{
	Block *block;
	Uint8 pos0[] = { 0, 0, 0, 0, 
			 0, 3, 4, 0,
			 0, 5, 6, 0,
			 0, 0, 0, 0 };

	block = block_new(4, 1);
	block_set_position(block, 0, pos0);

	block->type = BLOCK_TYPE_SQUARE;

	return block;
}


Block *
block_new_zee()
{
	Block *block;
	Uint8 pos0[] = {  0,  0,  0, 
			  9, 15,  0,
			  0, 16, 10 };
	Uint8 pos1[] = {  0,  0,  7,
			  0, 17, 18,
			  0,  8,  0 };

	block = block_new(3, 2);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);

	block->type = BLOCK_TYPE_ZEE;

	return block;
}


Block *
block_new_ess()
{
	Block *block;
	Uint8 pos0[] = {  0,  0,  0,
			  0, 17, 10,
			  9, 18,  0 };
	Uint8 pos1[] = {  0,  7,  0,
			  0, 16, 15,
			  0,  0,  8 };

	block = block_new(3, 2);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);

	block->type = BLOCK_TYPE_ESS;

	return block;
}


Block *
block_new_bar()
{
	Block *block;
	Uint8 pos0[] = {  0,  0,  0,  0,
			  0,  0,  0,  0,
			  9, 19, 19, 10,
			  0,  0,  0,  0 };
	Uint8 pos1[] = {  0,  7,  0,  0,
			  0, 20,  0,  0,
			  0, 20,  0,  0,
			  0,  8,  0,  0 };

	block = block_new(4, 2);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);

	block->type = BLOCK_TYPE_BAR;

	return block;
}


Block *
block_new_ell()
{
	Block *block;
	Uint8 pos0[] = {  0,  0,  0,
			 17, 19, 10,
			  8,  0,  0 };
	Uint8 pos1[] = {  9, 15,  0,
			  0, 20,  0,
			  0,  8,  0 };
	Uint8 pos2[] = {  0,  0,  7,
			  9, 19, 18,
			  0,  0,  0 };
	Uint8 pos3[] = {  0,  7,  0,
			  0, 20,  0,
			  0, 16, 10 };
 
	block = block_new(3, 4);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

	block->type = BLOCK_TYPE_ELL;

	return block;
}


Block *
block_new_jay()
{
	Block *block;
	Uint8 pos0[] = {  0,  0,  0,
			  9, 19, 15,
			  0,  0,  8 };
	Uint8 pos1[] = {  0,  7,  0,
			  0, 20,  0,
			  9, 18,  0 };
	Uint8 pos2[] = {  7,  0,  0,
			 16, 19, 10,
			  0,  0,  0 };
	Uint8 pos3[] = {  0, 17, 10,
			  0, 20,  0,
			  0,  8,  0 };

	block = block_new(3, 4);
	block_set_position(block, 0, pos0);
	block_set_position(block, 1, pos1);
	block_set_position(block, 2, pos2);
	block_set_position(block, 3, pos3);

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
	SDL_Rect sr;
	SDL_Rect dr;
	Uint8 x, y, i;
	Uint8 *pos = block->positions[block->current_position];
	
	/* All blocks are fixed size. Set DestRect and SourceRect. */
	dr.w = sr.w = BSIZE;
	dr.h = sr.w = BSIZE;
	sr.y = 0;

	s = SDL_CreateRGBSurface(0, block->size * BSIZE, block->size * BSIZE,
		screen->format->BitsPerPixel, 0, 0, 0, 0);

	for (y = 0; y < block->size; y++) {
		for (x = 0; x < block->size; x++) {
			i = y * block->size + x;
			if (pos[i]) {
				dr.x = x * BSIZE;
				dr.y = y * BSIZE;
				sr.x = pos[i] * BSIZE;
				SDL_BlitSurface(btex, &sr, s, &dr);
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

