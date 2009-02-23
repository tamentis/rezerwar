#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"

#include "rezerwar.h"


extern SDL_Surface *screen;
extern SDL_Surface *sprites;
extern Uint32 key;

#define A_SKY_SPEED 200

SDL_Surface *skytex = NULL;
Uint32 a_sky_offset = 0;
Uint32 a_sky_last = 0;


void
a_sky_update(Board *board, Uint32 now)
{
	if ((a_sky_last + A_SKY_SPEED) < now) {
		a_sky_offset++;
		a_sky_last = now;
	}
}


void
a_sky_refresh(Board *board)
{
	if (skytex == NULL)
		skytex = SDL_LoadBMP("gfx/a_sky/sky.bmp");

	SDL_Rect dst, src;
	src.w = screen->w;
	src.h = screen->h;
	src.x = a_sky_offset;
	src.y = 0;

	SDL_BlitSurface(skytex, &src, screen, NULL);
}
