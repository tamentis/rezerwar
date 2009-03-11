#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"

#include "rezerwar.h"


extern SDL_Surface *screen;
extern SDL_Surface *sprites;
extern Uint32 key;

#define A_SKY_SPEED 150

SDL_Surface *skytex = NULL;
uint32_t a_sky_offset = 0;
uint32_t a_sky_last = 0;


void
a_sky_update(Board *board, uint32_t now)
{
	if ((a_sky_last + A_SKY_SPEED) < now) {
		a_sky_offset++;
		a_sky_last = now;
	}

	if (skytex && a_sky_offset >= skytex->w)
		a_sky_offset = 0;
}


void
a_sky_refresh(Board *board)
{
	SDL_Rect src, dst;

	if (skytex == NULL)
		skytex = SDL_LoadBMP("gfx/a_sky/sky.bmp");

	src.w = screen->w;
	src.h = screen->h;
	src.x = a_sky_offset;
	src.y = 0;

	SDL_BlitSurface(skytex, &src, screen, NULL);
	if (a_sky_offset + screen->w > skytex->w) {
		dst.w = screen->w;
		dst.h = screen->h;
		dst.x = skytex->w - a_sky_offset;
		dst.y = 0;
		SDL_BlitSurface(skytex, NULL, screen, &dst);
	}
}
