#include <stdio.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


extern SDL_Surface *screen;


WaterOutput *
wateroutput_new(Uint8 size, Sint16 x, Sint16 y)
{
	WaterOutput *wo;

	wo = r_malloc(sizeof(WaterOutput));

	wo->size = size;
	wo->flow = 500;
	wo->x = x;
	wo->y = y;

	return wo;
}


void
wateroutput_kill(WaterOutput *wo)
{
	r_free(wo);
}


void
wateroutput_update(WaterOutput *wo, Board *board, Uint32 now)
{
	int i;

	if (wo->last_drop >= (now - wo->flow))
		return;

	wo->last_drop = now;
	for (i = 0; i < wo->size; i++) {
		board_launch_new_drop(board, wo->x + i, wo->y);
	}
}
