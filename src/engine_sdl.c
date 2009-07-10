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

void
init_gfx()
{
	uint32_t sdl_flags = 0;

	sdl_flags  = SDL_SWSURFACE;
//	sdl_flags  = SDL_HWSURFACE|SDL_DOUBLEBUF;
	sdl_flags |= conf->fullscreen == true ? SDL_FULLSCREEN : 0;
	screen = SDL_SetVideoMode(640, 480, 16, sdl_flags);
}

void
r_setpixel(Uint16 x, Uint16 y, Uint8 r, Uint8 g, Uint8 b)
{
	Uint32 *v32;
	Uint32 c32;

	switch (screen->format->BitsPerPixel) {
		case 8:
			printf("8bpp\n");
			break;
		case 16:
			printf("16bpp\n");
			break;
		case 24:
			printf("24bpp\n");
			break;
		case 32:
			c32 = SDL_MapRGBA(screen->format, r, g, b, 100);
//			c32 = SDL_MapRGB(screen->format, r, g, b);
//			printf("32bpp @ %dx%d=%d -> %d\n", x, y, y * screen->w * 4 + x, c32);
//			v32 = (Uint32*)screen->pixels + y * screen->w + x;
			v32 = screen->pixels + (y * screen->w + x) * 4;
			*v32 = c32;
			break;
		default:
			printf("oops... unknown pixel format.\n");
			exit(-1);
	}
}


void
r_setline(Uint16 x, Uint16 y, Uint16 w, Uint8 r, Uint8 g, Uint8 b)
{
	Uint16 i;

	/* Don't bother for single drops. */
	if (w == 1) {
		r_setpixel(x, y, r, g, b);
		return;
	}

	x = x - w / 2;

	for (i = 0; i < w; i++) {
		r_setpixel(x + i, y, r, g, b);
	}
}


void
white_screen()
{
	memset(screen->pixels, 255, screen->w * screen->h *
			screen->format->BytesPerPixel);
	SDL_Flip(screen);
}


void
black_screen(Uint8 s)
{
	memset(screen->pixels, 0, screen->w * screen->h *
			screen->format->BytesPerPixel);
	SDL_Flip(screen);
	SDL_Delay(s * 1000);
}

/**
 * Take a surface and convert each pixel into a greyscaled equivalent.
 */
void
surface_greyscale(SDL_Surface *s)
{
	int i, max = s->w * s->h;
	byte r, g, b, v;
	Uint32 *c32;
	Uint16 *c16;

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

void
surface_shutter(int start, int stop, int speed)
{
	SDL_Surface *org;
	SDL_Rect top, bottom;
	int i;

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

	/* Loop 'max' times and every time, dump first the original and then
	 * a increasingly transparent 'surf' */
	for (i = start; start < stop ? i < stop : i > stop; i += speed) {
		/* Only need to blit the original when we open. */
		if (start > stop)
			SDL_BlitSurface(org, NULL, screen, NULL);
		SDL_FillRect(screen, &top, 0);
		SDL_FillRect(screen, &bottom, 0);

		top.h = i;
		bottom.h = i;
		bottom.y -= speed;

		SDL_Flip(screen);
		SDL_Delay(10);
	}

	if (start > stop)
		SDL_FreeSurface(org);
}

void
surface_shutter_close()
{
	surface_shutter(0, screen->h / 2 + 20, 16);
}

void
surface_shutter_open()
{
	surface_shutter(screen->h / 2 + 20, 0, -16);
}

SDL_Surface *
new_of_size(SDL_Surface *s)
{
	SDL_Surface *org;
	org = SDL_CreateRGBSurface(0, s->w, s->h, s->format->BitsPerPixel,
			0, 0, 0, 0);
	return org;
}

SDL_Surface *
surface_subsample(SDL_Surface *org, int factor)
{
	SDL_Surface *pix = new_of_size(org);
	unsigned ssize = org->w * org->h;
	unsigned i, j, bpp;
	Uint32 *v;
	Uint8 r, g, b;

	bpp = org->format->BytesPerPixel;

	for (i = 0; i < ssize; i += factor) {
		v = org->pixels + i * bpp;
		SDL_GetRGB(*v, org->format, &r, &g, &b);
		for (j = 0; j < factor; j++) {
			v = pix->pixels + (i + j) * bpp;
			*v = SDL_MapRGB(org->format, r, g, b);
		}
	}

	return pix;
}

/**
 * Progressively sub-sample the original surface.
 */
void
surface_pixel_close()
{
	SDL_Surface *org, *pix;
	int psize;

	org = copy_screen();

	/* Number of frame, increase of one pixel each time... */
	for (psize = 1; psize < 65; psize <<= 1) {
		pix = surface_subsample(org, psize);
		SDL_BlitSurface(pix, NULL, screen, NULL);
		SDL_FreeSurface(pix);
		SDL_Flip(screen);
		SDL_Delay(40);
	}

	SDL_FreeSurface(org);
}

/**
 * Progressively sub-sample the original surface.
 */
void
surface_pixel_open()
{
	SDL_Surface *org, *pix;
	int psize;

	org = copy_screen();

	/* Number of frame, increase of one pixel each time... */
	for (psize = 64; psize > 0; psize >>= 1) {
		pix = surface_subsample(org, psize);
		SDL_BlitSurface(pix, NULL, screen, NULL);
		SDL_FreeSurface(pix);
		SDL_Flip(screen);
		SDL_Delay(40);
	}

	SDL_FreeSurface(org);
}

/**
 * Create a copy of the screen and return it as a new SDL_Surface, don't
 * forget to free it after usage.
 */
SDL_Surface *
copy_screen()
{
	SDL_Surface *org = new_of_size(screen);

	SDL_BlitSurface(screen, NULL, org, NULL);

	return org;
}


/**
 * Fade a surface in the screen. If any event occur, return 1 if the fade was
 * aborted by a keystroke.
 */
int
surface_fadein(SDL_Surface *surf, int speed)
{
	Uint8 i;
	int r = 0;
	SDL_Event event;
	SDL_Surface *org;
	int max = 255 / speed;

	/* Create a dump of the current screen to fade from */
	org = copy_screen();

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

void
blit_cursor(int style, int x, int y)
{
	SDL_Rect src, dest;

	src.w = src.h = dest.w = dest.h = 24;

	src.x = 270 + 24 * style;
	src.y = 174;

	dest.x = x - 12;
	dest.y = y - 2;

	SDL_BlitSurface(sprites, &src, screen, &dest);
}

/**
 * Dump a faded black surface on the screen.
 */
void
blit_modal(unsigned opacity)
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
 * Fade a surface out of the screen (to black). If a keystroke is recorded,
 * return 1.
 */
int
surface_fadeout(SDL_Surface *surf)
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

