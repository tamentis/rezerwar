#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"

#include "rezerwar.h"


extern SDL_Surface *screen;
extern SDL_Surface *sprites;

#define A_CHIMNEY_SIZE 36
#define A_CHIMNEY_SPEED 250

int a_chimneys_offsets[] = { 0, 1, 2, 1 };
int a_chimneys_status = 0;
uint32_t a_chimneys_last = 0;


void
a_chimneys_update(Board *board, uint32_t now)
{
	if (now < (a_chimneys_last + A_CHIMNEY_SPEED))
		return;

	a_chimneys_last = now;
	a_chimneys_status++;
	if (a_chimneys_status > 3)
		a_chimneys_status = 0;
}


void
a_chimneys_refresh(Board *board)
{
	SDL_Rect dst, src;
	dst.w = src.w = A_CHIMNEY_SIZE;
	dst.h = src.h = A_CHIMNEY_SIZE;
	dst.x = 316;
	dst.y = 36;

	src.x = 160 + a_chimneys_offsets[a_chimneys_status] * A_CHIMNEY_SIZE;
	src.y = 167;

	SDL_BlitSurface(sprites, &src, screen, &dst);
}
