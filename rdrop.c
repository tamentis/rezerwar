#include <stdio.h>

#include "SDL.h"

#include "rezerwar.h"


extern SDL_Surface *screen;


Drop *
drop_new(Sint16 x, Sint16 y)
{
	Drop *drop;

	drop = r_malloc(sizeof(Drop));

	drop->x = x;
	drop->y = y;

	drop->acc_x = 0.0;
	drop->acc_y = 1.0;

	drop->tick = 0;
	drop->moving = 1;

	return drop;
}


void
drop_kill(Drop *drop)
{
	r_free(drop);
}


void
drop_draw(Drop *drop, Uint16 offset_x, Uint16 offset_y)
{
	Uint16 x, y;

	x = drop->x + offset_x;
	y = drop->y + offset_y;

	r_setpixel(x, y, DROP_COLOR_R, DROP_COLOR_G, DROP_COLOR_B);
}

