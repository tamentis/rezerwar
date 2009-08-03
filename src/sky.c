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

#define SKY_SPEED	150

SDL_Surface *skytex = NULL;
uint32_t sky_offset = 0;
uint32_t sky_last = 0;


void
sky_update(Board *board, uint32_t now)
{
	if ((sky_last + SKY_SPEED) < now) {
		sky_offset++;
		sky_last = now;
	}

	if (skytex && sky_offset >= skytex->w)
		sky_offset = 0;
}


void
sky_render(Board *board)
{
	SDL_Rect src, dst;
	char *path;

	path = dpath("gfx/sky.bmp");

	if (skytex == NULL)
		skytex = SDL_LoadBMP(path);
	r_free(path);

	src.w = screen->w;
	src.h = screen->h;
	src.x = sky_offset;
	src.y = 0;

	SDL_BlitSurface(skytex, &src, screen, NULL);
	if (sky_offset + screen->w > skytex->w) {
		dst.w = screen->w;
		dst.h = screen->h;
		dst.x = skytex->w - sky_offset;
		dst.y = 0;
		SDL_BlitSurface(skytex, NULL, screen, &dst);
	}
}
