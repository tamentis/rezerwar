#include <stdio.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


extern SDL_Surface *screen;


void
board_random_output(Board *board)
{
	WaterOutput *wo;

	printf("board_random_output\n");

	wo = wateroutput_new(2, 100, 100);
	board_register_output(board, wo);
}


void
board_register_output(Board *board, WaterOutput *wo)
{
	size_t s;
	Uint8 i = board->output_count;

	board->output_count++;
	
	s = board->output_count * sizeof(WaterOutput);

	board->outputs = realloc(board->outputs, s);
	board->outputs[i] = wo;
}


void
board_update_outputs(Board *board, Uint32 now)
{
	Uint8 i;
	WaterOutput *wo;

	for (i = 0; i < board->output_count; i++) {
		wo = board->outputs[i];
		if (wo == NULL)
			continue;
		wateroutput_update(wo, board, now);
		
	}
}
