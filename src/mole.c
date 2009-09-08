/* $Id$
 *
 * Copyright (c) 2009 Bertrand Janin <tamentis@neopulsar.org>
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

SDL_Surface *mole_mask = NULL;
unsigned int mask_max = 0;


#define MOLE_WIDTH 	18
#define MOLE_HEIGHT	18

#define SPRITE_TOP	167
#define SPRITE_LEFT	197

#define DRILL_RATE	100
#define DRILL_OFFSET	MOLE_WIDTH

#define MOVE_RATE	400
#define MOVE_SIZE	2

#define HOLE_FIRST_X	160
#define HOLE_FIRST_Y	167
#define HOLE_FIRST_R	19
#define HOLE_SECOND_X	180
#define HOLE_SECOND_Y	169
#define HOLE_SECOND_R	15

#define HEADING_LEFT	mole->direction & 4
#define HEADING_RIGHT	mole->direction & 8

void mole_generate_valid_position(int *, int *);
void mole_load_mask();

Mole *
mole_new()
{
	Mole *mole;
	int i;

	mole = r_malloc(sizeof(Mole));

	mole_generate_valid_position(&mole->x, &mole->y);

	mole->drill_anim = 0;

	mole->drill_tick = 0;
	mole->move_tick = 0;

	mole->direction = 0;

	mole->trail_cur = -1;
	for (i = 0; i < MOLE_TRAIL; i++) {
		mole->trail_x[i] = -1;
		mole->trail_y[i] = -1;
	}

	mole->flooded = false;
	mole->trashed = false;

	mole->board = NULL;

	return mole;
}


void
mole_kill(Mole *mole)
{
	r_free(mole);
}


void
mole_update_drill(Mole *mole, uint32_t now)
{
	int rate = DRILL_RATE;

	if (mole->direction == 0)
		rate *= 8;

	if (now - rate < mole->drill_tick)
		return;

	if (mole->drill_anim == 0)
		mole->drill_anim = DRILL_OFFSET;
	else
		mole->drill_anim = 0;

	mole->drill_tick = now;
}


int
mole_get_vertical_offset(Mole *mole)
{
	if (mole->direction & 1)
		return - MOVE_SIZE;
	else if (mole->direction & 2)
		return MOVE_SIZE;

	return 0;
}

int
mole_get_horizontal_offset(Mole *mole)
{
	if (mole->direction & 4)
		return - MOVE_SIZE;
	else if (mole->direction & 8)
		return MOVE_SIZE;

	return 0;
}


/**
 * Check if the direction conflicts with the mask.
 */
bool
mole_move_check(Mole *mole)
{
	char *v1, *v2;
	int index;
	int x, y;

	mole_load_mask();

	x = mole->x + mole_get_horizontal_offset(mole);
	y = mole->y + mole_get_vertical_offset(mole);

	if (x > (mole_mask->w - MOLE_WIDTH) || x < 0)
		return false;

	if (y > (mole_mask->h - MOLE_WIDTH) || y < 0)
		return false;

	index = mole_mask->w * y + x;
	v1 = mole_mask->pixels + index;
	v2 = mole_mask->pixels + index + MOLE_WIDTH;
	if (*v1 == 0 && *v2 == 0)
		return true;

	return false;
}


void
mole_update_position(Mole *mole, uint32_t now)
{
	int cursor;
	int change_direction = false;

	/* Flooded moles do not move */
	if (mole->flooded == true)
		return;

	/* Ticker management */
	if (now - MOVE_RATE < mole->move_tick)
		return;
	mole->move_tick = now;

	/* A sleeping mole has less chance to move */
	if (mole->direction == 0)
		change_direction = rand() % 1000 > 990 ? true : false;
	else
		change_direction = rand() % 1000 > 940 ? true : false;

	/* From time to time, change direction */
	if (change_direction) {
		mole->direction  = rand() % 3;		// vertical
		mole->direction |= rand() % 3 << 2;	// horizontal
	}

	/* If we can move, apply the offsets */
	if (mole_move_check(mole) == true) {
		mole->y += mole_get_vertical_offset(mole);
		mole->x += mole_get_horizontal_offset(mole);
	} else {
		mole->direction >>= 1;
	}

	/* If the mole is reaching a pipe, boom, flood */
	if (mole->y < 418) {
		/* Right side */
		if (HEADING_LEFT && mole->x > (screen->w/2) && mole->x <= 486)
			mole_destroys_right_pipe(mole);

		/* Left side */
		if (HEADING_RIGHT && mole->x < (screen->w/2) && mole->x >= 135)
			mole_destroys_left_pipe(mole);
	}


	/* Append to the trail */
	mole->trail_cur++;
	if (mole->trail_cur >= MOLE_TRAIL)
		mole->trail_cur = 0;
	cursor = mole->trail_cur;

	mole->trail_x[cursor] = mole->x;
	mole->trail_y[cursor] = mole->y;
}


void
mole_update(Mole *mole, uint32_t now)
{
	mole_update_drill(mole, now);
	mole_update_position(mole, now);
}


void
mole_get_sprite_rect(Mole *mole, SDL_Rect *r)
{
	r->w = MOLE_WIDTH;
	r->h = MOLE_HEIGHT;
	r->x = SPRITE_LEFT + mole->drill_anim;
	r->y = SPRITE_TOP;

	switch (mole->direction) {
		case 1: // up
		case 7:
			r->x += MOLE_WIDTH * 4;
			break;
		case 3:
		case 8: // right
		case 10:
		case 9:
			r->x += MOLE_WIDTH * 2;
			break;
		case 2: // down
			r->x += MOLE_WIDTH * 6;
			break;
		case 4: // left
		case 5:
		case 6:
			break;
		case 0: // zz zzz
			r->x += MOLE_WIDTH * 8;
			break;
		default:
			break;
	}
}


void
mole_get_hole_rects(SDL_Rect *first, SDL_Rect *second, SDL_Rect *dh1,
		SDL_Rect *dh2)
{
	first->w = first->h = dh1->w = dh1->h = HOLE_FIRST_R;
	second->w = second->h = dh2->w = dh2->h = HOLE_SECOND_R;

	first->x = HOLE_FIRST_X;
	first->y = HOLE_FIRST_Y;

	second->x = HOLE_SECOND_X;
	second->y = HOLE_SECOND_Y;
}


void
mole_get_rectangle(Mole *mole, SDL_Rect *r)
{
	r->w = MOLE_WIDTH;
	r->h = MOLE_HEIGHT;
	r->x = mole->x;
	r->y = mole->y;
}

void
mole_render(Mole *mole) 
{
	SDL_Rect src, dest;

	/* Flooded mole */
	if (mole->flooded == true)
		return;
	
	/* The mole itself */
	mole_get_sprite_rect(mole, &src);
	mole_get_rectangle(mole, &dest);
	SDL_BlitSurface(sprites, &src, screen, &dest);
}

void
mole_render_trail(Mole *mole) 
{
	SDL_Rect hole1, hole2;
	SDL_Rect dh1, dh2;
	int i;
	
	/* The trail, first layer */
	mole_get_hole_rects(&hole1, &hole2, &dh1, &dh2);

	/* Flooded pipe */
	if (mole->flooded == true) {
		hole2.y += 17;
	}

	for (i = 0; i < MOLE_TRAIL; i++) {
		if (mole->trail_x[i] == -1)
			continue;

		dh1.x = mole->trail_x[i] - 1;
		dh1.y = mole->trail_y[i] - 3;
		SDL_BlitSurface(sprites, &hole1, screen, &dh1);
	}

	for (i = 0; i < MOLE_TRAIL; i++) {
		if (mole->trail_x[i] == -1)
			continue;

		dh2.x = mole->trail_x[i] + 1;
		dh2.y = mole->trail_y[i] - 1;
		SDL_BlitSurface(sprites, &hole2, screen, &dh2);
	}
}

void
mole_generate_valid_position(int *x, int *y)
{
	char *v1, *v2;
	unsigned int index;
	bool invalid = true;

	mole_load_mask();

	while (invalid) {
		index = (rand() * 10) % mask_max;
		v1 = mole_mask->pixels + index;
		v2 = mole_mask->pixels + index + MOLE_WIDTH;
		if (*v1 == 0 && *v2 == 0)
			invalid = false;
	}

	*y = index / mole_mask->w;
	*x = index % mole_mask->w;
}


void
mole_load_mask()
{
	char *path;

	if (mole_mask == NULL) {
		path = dpath("gfx/molemask.bmp");
		mole_mask = SDL_LoadBMP(path);
		r_free(path);
		if (mole_mask == NULL)
			fatal("Unable to load mole mask.");
		mask_max = mole_mask->w * mole_mask->h - MOLE_WIDTH;
	}
}


/**
 * Called when a mole hits a pipe by the left side.
 */
void
mole_destroys_left_pipe(Mole *mole)
{
	int index = (mole->y - BOARD_TOP) / BSIZE;
	mole->board->pipes[index]->status = 0;
	mole->board->pipes[index]->mole = mole;
	mole->flooded = true;
	sfx_play_splash();
}


void
mole_destroys_right_pipe(Mole *mole)
{
	int index = (mole->y - BOARD_TOP) / BSIZE;
	mole->board->pipes[BOARD_HEIGHT+index]->status = 0;
	mole->board->pipes[BOARD_HEIGHT+index]->mole = mole;
	mole->flooded = true;
	sfx_play_splash();
}

