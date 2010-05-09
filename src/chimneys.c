/*
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

#define CHIMNEY_SIZE	56
#define CHIMNEY_SPEED	250

int chimneys_offsets[] = { 0, 1, 2, 1 };
int chimneys_status = 0;
uint32_t chimneys_last = 0;


/**
 * Internal update at every tick.
 */
void
chimneys_update(Board *board, uint32_t now)
{
	if (now < (chimneys_last + CHIMNEY_SPEED))
		return;

	chimneys_last = now;
	chimneys_status++;
	if (chimneys_status > 3)
		chimneys_status = 0;
}


/**
 * Draw for each frame.
 */
void
chimneys_render(Board *board)
{
	SDL_Rect dst, src;
	dst.w = src.w = CHIMNEY_SIZE;
	dst.h = src.h = CHIMNEY_SIZE;
	dst.x = 313;
	dst.y = 18;

	src.x = 160 + chimneys_offsets[chimneys_status] * CHIMNEY_SIZE;
	src.y = 241;

	SDL_BlitSurface(sprites, &src, screen, &dst);
}
