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


#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"

#include "rezerwar.h"


extern SDL_Surface *screen;
extern SDL_Surface *sprites;
extern Uint32 key;


Pipe *
pipe_new()
{
	Pipe *pipe;

	pipe = r_malloc(sizeof(Pipe));

	pipe->tick = 0;
	pipe->status = -1;
	pipe->mole = NULL;

	return pipe;
}


/**
 * Render the pipe statuses
 */
void
board_render_pipes(Board *board)
{
	int i;
	int offsets[] = {
		0, 
		PIPE_SPRITE_SIZE,
		PIPE_SPRITE_SIZE * 2,
		PIPE_SPRITE_SIZE
	};
	SDL_Rect src, dest;

	/* Positions in the sprite file */
	src.x = PIPE_SPRITE_LEFT;
	src.y = PIPE_SPRITE_TOP;
	src.w = dest.w = PIPE_SPRITE_SIZE;
	src.h = dest.h = PIPE_SPRITE_SIZE;

	for (i = 1; i < BOARD_HEIGHT * 2; i++) {
		if (board->pipes[i]->status != -1) {
			src.x = 161 + offsets[board->pipes[i]->status];
			if (i < BOARD_HEIGHT) { // left
				dest.y = BOARD_TOP + i * BSIZE - 4;
				dest.x = BOARD_LEFT - BSIZE;
			} else { // right
				dest.y = BOARD_TOP + (i - BOARD_HEIGHT) * BSIZE - 4;
				dest.x = BOARD_LEFT + BSIZE * BOARD_WIDTH - 4;
			}
			SDL_BlitSurface(sprites, &src, screen, &dest);
		}
	}
}

void
pipe_kill(Pipe *pipe)
{
	r_free(pipe);
}

void
pipe_fix(Board *board, Pipe *pipe)
{
	pipe->status = -1;

	if (pipe->mole != NULL) {
		pipe->mole->trashed = true;
		board_add_points(board, POINTS_FIX_PIPE, pipe->mole->x, 
					pipe->mole->y);
	}
}
