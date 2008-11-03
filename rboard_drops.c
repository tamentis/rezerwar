#include <stdio.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


extern SDL_Surface *screen;


Uint16
board_add_drop(Board *b, Drop *p)
{
	Uint16 c;

	b->drop_count++;

	b->drops = realloc(b->drops,
			b->drop_count * sizeof(Drop *));

	c = b->drop_count - 1;

	b->drops[c] = p;

	return c;
}


void
board_launch_new_drop(Board *board, Sint16 x, Sint16 y)
{
	Drop *p = drop_new(x, y);
	board_add_drop(board, p);
}


void
board_refresh_drops(Board *board)
{
	Uint16 i;
	Drop *drop;

	for (i = 0; i < board->drop_count; i++) {
		drop = board->drops[i];
		if (drop == NULL)
			continue;
		
		drop_draw(drop, board->offset_x, board->offset_y);
	}
}


/**
 * Returns the type of the pixel area defined by the x;y coordinates.
 */
Uint8
board_get_pixel_area_type(Board *board, Sint16 x, Sint16 y)
{
	Uint16 p;
	Sint16 cx, cy;
	Uint8 type;

	/* Left, right and bottom of the board. */
	if (x < 0)
		return ATYPE_BOARD_LEFT;
	if (x >= BSIZE * board->width)
		return ATYPE_BOARD_RIGHT;
	if (y >= BSIZE * board->height)
		return ATYPE_BOARD_BOTTOM;

	/* Block in the way. */
	board_update_map(board);
	cx = x / BSIZE;
	cy = y / BSIZE;
	type = board->map[cy * board->width + cx];
	if (type == 1)
		return ATYPE_BLOCK;
	    
	/* index of the pixel */
	p = y * board->width * BSIZE + x;

	/* Drop underneath! */
	if (board->drop_map[p])
		return ATYPE_DROP;

	return ATYPE_FREE;
}


/**
 * Return the pixel area type using a drop and relative offsets.
 */
Uint8
board_drop_check_move(Board *board, Drop *drop, Sint8 x, Sint8 y)
{
	return board_get_pixel_area_type(board, drop->x + x, drop->y + y);
}


/**
 * Flush the whole map and add all the moving drops.
 */
void
board_update_drop_map(Board *board)
{
	Uint16 i, p;
	Drop *drop;

	/* Set all the pointers of the map to zero. */
	for (i = 0; i < board->drop_map_size; i++) {
		board->drop_map[i] = NULL;
	}

	for (i = 0; i < board->drop_count; i++) {
		drop = board->drops[i];
		if (drop == NULL)
			continue;
		if (drop->moving)
			continue;

		p = drop->y * board->width * BSIZE + drop->x;
		board->drop_map[p] = drop;
	}
}



/* board_dump_drop_map() - used for debug purposes, this will spit out on the
 * console the current status of the drop map. size is the size in pixels of 
 * the drop map itself. */
void
board_dump_drop_map(Board *board)
{
	Uint16 i;
	Uint16 size = board->width * board->height * BSIZE * BSIZE;

	printf("Dumping board's drop_map.\n");

	for (i = 0; i < size; i++) {
		printf("%c", board->drop_map[i] ? 'o' : ' ');
		if (i % (board->width * BSIZE))
			printf("\n");
	}
}


/**
 * Dump the drop map as a raw bitmap, save the file as '/tmp/dropmap.dump'
 */
void
board_dump_drop_map_bmp(Board *board)
{
	Uint16 i;
	Uint16 size = board->width * board->height * BSIZE * BSIZE;
	unsigned char *buf;
	FILE *fp;
	size_t s;

	printf("Dumping board's drop_map as '/tmp/dropmap.dump' (%dx%d).\n",
			board->width * BSIZE, board->height * BSIZE);

	buf = malloc(size);
	for (i = 0; i < size; i++)
		buf[i] = board->drop_map[i] ? 0xFF : 0x00;

	fp = fopen("/tmp/dropmap.dump", "wb");
	s = fwrite(buf, size, 1, fp);
	fclose(fp);
	free(buf);
}


Uint16
board_get_drop_index(Board *board, Drop *drop)
{
	return (drop->y);
}


/* board_find_drop_space() - this function will determine where a drop should
 * be placed when it has been determined that it was falling on another drop.
 */
void
board_find_drop_space(Board *board, Drop *drop)
{
	Uint8 type;
	Sint16 i;
	Sint16 ox;
	const Sint16 oxmax = board->width * BSIZE;
	Sint16 min = 0;
	Sint16 max = oxmax;
	Uint8 reached_left = 0, reached_right = 0;
	
	/* This crazy for will have ox increase of one alternatively negative
	 * and positive. In clear: 1 -1 2 -2 3 -3 4 -4, etc.. */
	for (ox = 1; ox < oxmax; ox < 0 ? ox-- : 1, ox *= -1) {
		type = board_drop_check_move(board, drop, ox, +1);
		i = drop->x + ox;

		/* Ignore out of limit stuff. */
		if (i < min) continue;
		if (i > max) continue;

		/* Found free slot within the range. */
		if (type == ATYPE_FREE && i > min && i < max) {
			drop->y++;
			drop->x = i;
			return;
		}


		/* If we found a blocker that is not a drop, update the min
		 * and max accordingly. */
		if (type != ATYPE_DROP) {
			if (ox < 0) {
				min = i;
				reached_left = 1;
			} else {
				max = i;
				reached_right = 1;
			}
		}

		/* If we already touched borders on both sides, forget about
		 * it and don't move the drop, it's fine where it is. */
		if (reached_left && reached_right) {
			drop->moving = 0;
			board_update_drop_map(board);
			return;
		}
	}
	
	/* p is the index of the drop underneath. */
//	(*y)++;
//	(*x)--;
}


/**
 * Update a single drop's position according to the environement.
 */
void
board_update_single_drop(Board *board, Drop *drop)
{
	Uint8 c;

	c = board_drop_check_move(board, drop, 0, +1);

	switch (c) {
		case ATYPE_FREE:
			drop->y++;
			break;

		case ATYPE_BLOCK:
		case ATYPE_BOARD_BOTTOM:
			drop->moving = 0;
			board_update_drop_map(board);
			break;

		case ATYPE_DROP:
			board_find_drop_space(board, drop);
			break;
	}
}


/**
 * Update the positionning of all the drops on the board, basically calls
 * update_single_drop in chain on all the moving drops.
 */
void
board_update_drops(Board *board, Uint32 now)
{
	Uint16 i;
	Drop *drop;

	for (i = 0; i < board->drop_count; i++) {
		drop = board->drops[i];
		if (drop == NULL)
			continue;

		if (drop->moving && (now - drop->tick > board->drop_speed)) {
			board_update_single_drop(board, drop);
			drop->tick = now;
		}
	}
}

