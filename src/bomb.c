/*
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


/*
 * Everything relative to cube bombs and their flames (explosions).
 */


#include <stdio.h>

#include "SDL.h"

#include "rezerwar.h"

extern SDL_Surface *screen;
extern SDL_Surface *sprites;
extern Uint32 key;

#define BOOM_SPEED	50
#define MAX_STATES	7

int flame_offsets[MAX_STATES] = { 2, 1, 0, 0, 1, 2, 3 };

/* Alignment */
enum {
	HORIZONTAL,
	VERTICAL,
};


/**
 * A Cube Bomb just dropped! Execute its explosion and clear it up!
 */
void
board_cube_bomb(Board *board, Cube *cube)
{
	/* Bomb shouldn't stop combos from happening, they are helpers */
	board->settled = false;

	if (cube->current_position % 2 == 0) {
		board_kill_row(board, cube->y);
		board_spawn_flame(board, HORIZONTAL, cube->y);
	} else {
		board_kill_column(board, cube->x);
		board_spawn_flame(board, VERTICAL, cube->x);
	}

	sfx_play_boom();
}


/**
 * Initial setup on the board (called from board_new)
 */
void
board_initialize_flames(Board *board)
{
	int i;

	board->last_flame = -1;
	for (i = 0; i < MAX_FLAMES; i++)
		board->flames[i] = NULL;
}


/**
 * Game Internals Update for Flames.
 */
void
board_update_flames(Board *board, uint32_t now)
{
	int	 i;
	Flame	*flame;

	for (i = 0; i < MAX_FLAMES; i++) {
		flame = board->flames[i];

		if (flame == NULL)
			continue;

		/* Not to be updated... yet */
		if ((now - flame->tick) < BOOM_SPEED)
			continue;

		/* Loop sprite states */
		flame->state++;

		/* End of cycle... */
		if (flame->state >= MAX_STATES) {
			board->flames[i] = NULL;
			flame_kill(flame);
		}

		flame->tick = now;
	}
}


/**
 * Rendering of all the flames.
 */
#define FLAME_LEFT	160
#define FLAME_TOP	339
void
board_render_flames(Board *board)
{
	int		 i, j;
	SDL_Rect	 source, dest;
	Flame		*flame;

	source.w = dest.w = source.h = dest.h = BSIZE;


	for (i = 0; i < MAX_FLAMES; i++) {
		flame = board->flames[i];

		if (flame == NULL)
			continue;

		source.y = FLAME_TOP + flame_offsets[flame->state] * BSIZE;

		if (flame->type == VERTICAL) {
			source.x = FLAME_LEFT + BSIZE;
			dest.x = BOARD_LEFT + flame->pos * BSIZE;
			for (j = 1; j < BOARD_HEIGHT; j++) {
				dest.y = BOARD_TOP + j * BSIZE;
				gfx_blitsprite(&source, &dest);
			}

		} else /* HORIZONTAL */ {
			source.x = FLAME_LEFT;
			dest.y = BOARD_TOP + flame->pos * BSIZE;
			for (j = 0; j < BOARD_WIDTH; j++) {
				dest.x = BOARD_LEFT + j * BSIZE;
				gfx_blitsprite(&source, &dest);
			}
		}


	}
}


/**
 * Flame constructor
 */
Flame *
flame_new(int type, int pos)
{
	Flame *flame;

	flame = r_malloc(sizeof(Flame));
	flame->type = type;
	flame->pos = pos;
	flame->state = -1;
	flame->tick = 0;

	return flame;
}


/**
 * Flame destructor
 */
void
flame_kill(Flame *flame)
{
	r_free(flame);
}


/**
 * Spawn Flames in a board within the given pool of MAX_FLAMES pointers.
 */
void
board_spawn_flame(Board *board, int type, int pos)
{
	Flame	*flame;

	board->last_flame++;

	/* Loop */
	if (board->last_flame >= MAX_FLAMES)
		board->last_flame = 0;

	/* If the new slot is busy, kill the previous one. */
	flame = board->flames[board->last_flame];
	if (flame != NULL)
		flame_kill(flame);

	board->flames[board->last_flame] = flame_new(type, pos);
}


