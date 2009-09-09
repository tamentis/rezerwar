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

#include "SDL.h"

#include "rezerwar.h"


extern SDL_Surface *screen;
extern SDL_Surface *sprites;
extern Uint32 key;
extern Configuration *conf;


/**
 * Initialize the video sub-system.
 */
void
gfx_init()
{
	uint32_t sdl_flags = 0;

//	sdl_flags  = SDL_SWSURFACE;
	sdl_flags  = SDL_HWSURFACE|SDL_DOUBLEBUF;
	sdl_flags |= conf->fullscreen == true ? SDL_FULLSCREEN : 0;
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, sdl_flags);
}


/**
 * Fill the given surface with black.
 */
void
gfx_black(SDL_Surface *surf)
{
	memset(surf->pixels, 0, surf->w * surf->h *
			surf->format->BytesPerPixel);
}


/**
 * Take a surface and convert each pixel into a greyscaled equivalent.
 */
void
gfx_greyscale(SDL_Surface *s)
{
	int	 i, max = s->w * s->h;
	byte	 r, g, b, v;
	Uint32	*c32;
	Uint16	*c16;

	if (s->format->BitsPerPixel == 16) {
		for (i = 0; i < max; i++) {
			c16 = s->pixels + i * s->format->BytesPerPixel;
			if (*c16 == key)
				continue;
			SDL_GetRGB(*c16, s->format, &r, &g, &b);
			v = (r + g + b) / 3;
			*c16 = SDL_MapRGB(s->format, v, v, v);
		}
	} else {
		for (i = 0; i < max; i++) {
			c32 = s->pixels + i * s->format->BytesPerPixel;
			if (*c32 == key)
				continue;
			SDL_GetRGB(*c32, s->format, &r, &g, &b);
			v = (r + g + b) / 3;
			*c32 = SDL_MapRGB(s->format, v, v, v);
		}
	}
}


/**
 * Blit a given surface to screen at the given coordinates.
 */
void
gfx_toscreen(SDL_Surface *surf, int x, int y)
{
	SDL_Rect dest;

	dest.x = x;
	dest.y = y;
	dest.w = surf->w;
	dest.h = surf->h;

	SDL_BlitSurface(surf, NULL, screen, &dest);
}


void
gfx_free(SDL_Surface *surf)
{
	SDL_FreeSurface(surf);
}


void
gfx_shutter(int start, int stop, int speed)
{
	SDL_Surface	*org = NULL;
	SDL_Rect	 top, bottom;
	int		 i;
	uint32_t	 loop_start, elapsed;

	/* If the shutter is going backward, you need to dump the original
	 * screen every time */
	if (start > stop) {
		/* Create a dump of the current screen to fade from */
		org = SDL_CreateRGBSurface(0, screen->w, screen->h, 
				screen->format->BitsPerPixel, 0, 0, 0, 0);
		SDL_BlitSurface(screen, NULL, org, NULL);
	}

	top.w = screen->w;
	top.h = start;
	top.x = 0;
	top.y = 0;

	bottom.w = screen->w;
	bottom.h = start;
	bottom.x = 0;
	bottom.y = screen->h - start;

	/* 
	 * Loop 'max' times and every time, dump first the original and then
	 * a increasingly transparent 'surf'
	 */
	for (i = start; start < stop ? i < stop : i > stop; i += speed) {
		loop_start = SDL_GetTicks();
		/* Only need to blit the original when we open. */
		if (start > stop)
			SDL_BlitSurface(org, NULL, screen, NULL);
		SDL_FillRect(screen, &top, 0);
		SDL_FillRect(screen, &bottom, 0);

		top.h = i;
		bottom.h = i;
		bottom.y -= speed;

		SDL_Flip(screen);
		elapsed = SDL_GetTicks() - loop_start;
		if (elapsed < 20)
			SDL_Delay(20 - elapsed);
	}

	if (start > stop)
		SDL_FreeSurface(org);
}

/**
 * Grow a black border up and down around the current image.
 */
void
gfx_shutter_close()
{
	gfx_shutter(0, screen->h / 2 + 20, 16);
}


/**
 * Start on black and open the shutter to reveal the image.
 */
void
gfx_shutter_open()
{
	gfx_shutter(screen->h / 2 + 20, 0, -16);
}


/**
 * Create a new surface with the given size.
 */
SDL_Surface *
gfx_new(int width, int height)
{
	SDL_Surface *surface;

	surface = SDL_CreateRGBSurface(0, width, height, 
			screen->format->BitsPerPixel, 0, 0, 0, 0);

	return surface;
}


/**
 * Create a new surface, using the same size and format as the given one.
 */
SDL_Surface *
gfx_new_samesize(SDL_Surface *s)
{
	SDL_Surface *org;
	org = SDL_CreateRGBSurface(0, s->w, s->h, s->format->BitsPerPixel,
			0, 0, 0, 0);
	return org;
}


/**
 * Create a copy of the screen and return it as a new SDL_Surface, don't
 * forget to free it after usage.
 */
SDL_Surface *
gfx_copyscreen()
{
	SDL_Surface *org = gfx_new_samesize(screen);

	SDL_BlitSurface(screen, NULL, org, NULL);

	return org;
}


/**
 * Fade a surface in the screen. If any event occur, return 1 if the fade was
 * aborted by a keystroke.
 */
int
gfx_fadein(SDL_Surface *surf, int speed)
{
	Uint8 i;
	int r = 0;
	SDL_Event event;
	SDL_Surface *org;
	int max = 255 / speed;

	/* Create a dump of the current screen to fade from */
	org = gfx_copyscreen();

	/* Loop 'max' times and every time, dump first the original and then
	 * a increasingly transparent 'surf' */
	for (i = 0; i < max; i++) {
		/* Skip 128 as it has a special meaning for SDL */
		if (i * speed == 128) continue;

		if (SDL_PollEvent(&event) && (event.type == SDL_KEYDOWN ||
					event.type == SDL_MOUSEBUTTONDOWN)) {
			r = 1;
			i = max;
		}

		memset(screen->pixels, 0, screen->w * screen->h *
				screen->format->BytesPerPixel);
		SDL_SetAlpha(surf, SDL_RLEACCEL | SDL_SRCALPHA, i * speed);
		SDL_BlitSurface(org, NULL, screen, NULL);
		SDL_BlitSurface(surf, NULL, screen, NULL);
		SDL_Flip(screen);
		SDL_Delay(10);
	}

	SDL_FreeSurface(org);

	return r;
}


/**
 * Fade a surface out of the screen (to black). If a keystroke is recorded,
 * return 1.
 */
int
gfx_fadeout(SDL_Surface *surf)
{
	Uint8 i;
	SDL_Event event;

	for (i = 0; i < 64; i++) {
		if (SDL_PollEvent(&event) && (event.type == SDL_KEYDOWN ||
					event.type == SDL_MOUSEBUTTONDOWN))
			return 1;

		memset(screen->pixels, 0, screen->w * screen->h *
				screen->format->BytesPerPixel);
		SDL_SetAlpha(surf, SDL_RLEACCEL | SDL_SRCALPHA, 255 - i * 4);
		SDL_BlitSurface(surf, NULL, screen, NULL);
		SDL_Flip(screen);
		SDL_Delay(10);
	}

	return 0;
}


/**
 * Dump a faded black surface on the screen.
 */
void
gfx_modal(unsigned opacity)
{
	SDL_Surface *m;

	m = SDL_CreateRGBSurface(0, screen->w, screen->h, 
			screen->format->BitsPerPixel, 0, 0, 0, 0);
	SDL_FillRect(m, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
	SDL_SetAlpha(m, SDL_SRCALPHA|SDL_RLEACCEL, opacity);
	SDL_BlitSurface(m, NULL, screen, NULL);

	SDL_FreeSurface(m);
}


/**
 * Blit a sprite to screen given a source and destination rectangle. This is
 * really a tiny convenience wrapper.
 */
void
gfx_blitsprite(SDL_Rect *source, SDL_Rect *dest)
{
	SDL_BlitSurface(sprites, source, screen, dest);
}

