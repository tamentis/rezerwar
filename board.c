#include <stdio.h>

#include "SDL.h"

#include "rezerwar.h"


extern Configuration *conf;
extern Board *board;
extern SDL_Surface *screen;
extern SDL_Surface *sprites;
extern Uint32 key;


/**
 * Board constructor
 */
Board *
board_new(int difficulty)
{
	Board *b;
	size_t size = BOARD_WIDTH * BOARD_HEIGHT;
	int i;

	b = r_malloc(sizeof(Board));

	b->width = BOARD_WIDTH;
	b->height = BOARD_HEIGHT;

	b->difficulty = difficulty;

	b->offset_x = BOARD_LEFT;
	b->offset_y = BOARD_TOP;

	b->bg = NULL;

	b->elapsed = 0;

	/* Cube related members initialization. */
	b->cube_count = 0;
	b->cubes = r_malloc(size * sizeof(Cube *));
	for (i = 0; i < size; i++)
		b->cubes[i] = NULL;

	/* Initialize the Block Queue */
	b->bqueue = NULL;
	b->bqueue_len = 0;

	/* Block related members. */
	b->block_count = 0;
	b->blocks = NULL;
	b->current_block = NULL;
	b->next_block = NULL;
	b->block_speed = SPEED_NORMAL;
	b->block_speed_factor = 1;
	b->remains = -1;
	b->launch_next = false;
	b->next_line = 1;
	b->rising_speed = NEXTLINE;
	b->time_limit = -1;

	/* Texts (future OSD) related members */
	b->texts = NULL;
	b->text_count = 0;

	/* Movement related (& controls) */
	b->moving_left = 0;
	b->moving_right = 0;
	b->lateral_speed = 100;
	b->lateral_tick = 0;

	/* Player related */
	b->score = 0;
	b->status = MTYPE_NOP;
	b->paused = false;
	b->gameover = false;
	b->success = false;
	b->silent = false;
	b->allow_dynamite = true;
	b->objective_type = OBJTYPE_NONE;

	/* Prompt init. */
	b->prompt_text = NULL;
	b->prompt_func = NULL;

	/* Modal and score */
	b->modal = false;
	b->score_t = board_add_text(b, "0", 10, 10);
	b->timeleft_t = board_add_text(b, "", 10, 30);

	/* Status message */
	b->status_t = board_add_text(b, "", 260, 240);
	b->status_t->effect = EFFECT_SHAKE;
	b->status_t->centered = true;
	text_set_color1(b->status_t, 225, 186, 0);
	text_set_color2(b->status_t, 127,  55, 0);

	/* (optional) FPS display */
	b->show_fps = false;
	b->fps_t = board_add_text(b, "", 550, 10);
	text_set_color1(b->status_t, 225, 40, 0);
	text_set_color2(b->status_t, 56,  8, 8);

	/* Load background. */
	b->bg = SDL_LoadBMP("gfx/gameback.bmp");
	SDL_SetColorKey(b->bg, SDL_SRCCOLORKEY|SDL_RLEACCEL, key);

	/* Level stuff */
	b->next_level = NULL;

	return b;
}


/**
 * Create a new board and populate the cubes from the given level.
 */
Board *
board_new_from_level(Level *level)
{
	int i, j;
	Board *board;
	Block *block = NULL;
	Cube *cube = NULL;
	Text *title, *description, *prompt;

	board = board_new(0);

	/* Transfer the cubes */
	for (i = 0; i < (BOARD_WIDTH * BOARD_HEIGHT); i++) {
		cube = cube_new_from_char(level->cmap[i]);
		if (cube == NULL)
			continue;
		cube->y = i / BOARD_WIDTH;
		cube->x = i % BOARD_WIDTH;
		board->cubes[i] = cube;
	
		/* No need to count rocks.. their passive. */
		if (cube->type == CTYPE_ROCK)
			continue;

		board->cube_count++;
	}

	/* Transfer the queue */
	board->bqueue_len = level->queue_len;
	board->bqueue = r_malloc(sizeof(Block *) * board->bqueue_len);
	for (i = 0; i < board->bqueue_len; i++) {
		block = block_new_of_type(level->queue[i]->type);
		block->current_position = level->queue[i]->pos;
		for (j = 0; j < level->queue[i]->cmap_len; j++) {
			cube_kill(block->cubes[j]);
			block->cubes[j] = cube_new_from_char(level->queue[i]->cmap[j]);
		}
		board->bqueue[i] = block;
	}

	/* Prepare the board to welcome the text */
	board->modal = true;
	board->silent = true;
	board->paused = true;

	/* Copy level related stuff */
	board->objective_type = level->objective_type;
	board->allow_dynamite = level->allow_dynamite;
	if (level->next)
		board->next_level = r_strcp(level->next);
	if (level->max_blocks)
		board->remains = level->max_blocks;
	board->rising_speed = level->rising_speed;
	board->time_limit = level->time_limit;

	/* Draw the title */
	title = board_add_text(board, level->name, 20, 20);
	title->centered = true;
	title->temp = true;
	text_set_colors(title, 0xFFE64B, 0xB35904);

	/* Draw the description */
	description = board_add_text(board, level->description, 20, 90);
	description->temp = true;
	description->font = 1;

	/* Draw the press p to contine */
	prompt = board_add_text(board, "press 'enter' to start", 350, 440);
	prompt->temp = true;
	text_set_colors(prompt, 0xFFE64B, 0xB35904);

	return board;
}

void
board_kill(Board *board)
{
	int i, size;

	/* Cube cleanup (only if cubes we have) */
	if (board->cubes != NULL) {
		size = board->width * board->height;
		for (i = 0; i < size; i++) {
			if (board->cubes[i] == NULL)
				continue;

			cube_kill(board->cubes[i]);
		}
		r_free(board->cubes);
		board->cubes = NULL;
	}


	/* Block clean up */
	if (board->next_block != NULL) {
		block_kill(board->next_block);
		board->next_block = NULL;
	}

	for (i = 0; i < board->block_count; i++) {
		if (board->blocks[i] == NULL)
			continue;
		block_kill(board->blocks[i]);
	}
	free(board->blocks);
	board->block_count = 0;
	board->blocks = NULL;

	/* Text cleanup */
	for (i = 0; i < board->text_count; i++) {
		if (board->texts[i] == NULL)
			continue;
		text_kill(board->texts[i]);
	}
	free(board->texts);

	/* Block queue cleanup */
	r_free(board->bqueue);
	board->bqueue = NULL;
	board->bqueue_len = 0;

	/* Level stuff */
	r_free(board->next_level);

	/* General board clean up */
	SDL_FreeSurface(board->bg);
	r_free(board);
}


Text *
board_add_text(Board *board, char *value, int x, int y)
{
	Text *t;

	t = text_new(value);
	
	board->texts = realloc(board->texts, (board->text_count + 1) * sizeof(Text*));
	board->texts[board->text_count] = t;
	board->text_count++;

	t->x = x;
	t->y = y;

	return t;
}


void
board_refresh_texts(Board *board)
{
	int i;
	Text *t;
	SDL_Rect r;
	SDL_Surface *s;

	/* Update the score Text */
	if (board->silent == true) {
		text_set_value(board->score_t, "");
	} else {
		char score[20];
		snprintf((char *)score, 20, "score: %d", board->score);
		text_set_value(board->score_t, score);
	}

	/* Update the time Text */
	if (board->time_limit > -1) {
		char tl[20];
		snprintf((char *)tl, 20, "time: %d", board->time_limit);
		text_set_value(board->timeleft_t, tl);
	}

	/* Add a modal under all the text. */
	if (board->modal == true)
		blit_modal(160);

	/* Draw all the Texts, cleaning up trashed ones. */
	for (i = 0; i < board->text_count; i++) {
		t = board->texts[i];
		if (t == NULL) continue;

		if (t->trashed == true) {
			text_kill(t);
			board->texts[i] = NULL;
			continue;
		}
		
		s = text_get_surface(t);

		/* It's legal for a text to have no surface, just skip it */
		if (s == NULL)
			continue;

		text_get_rectangle(t, &r);

		SDL_BlitSurface(s, NULL, screen, &r);
		SDL_FreeSurface(s);
	}
}

/**
 * Remove all the text marked 'temp'.
 */
void
board_trash_temp_texts(Board *board)
{
	Text *t;
	int i;

	for (i = 0; i < board->text_count; i++) {
		t = board->texts[i];
		if (t == NULL)
			continue;

		if (t->temp == true)
			t->trashed = true;
	}
}

/**
 * Toggle paused state. Ignore the request in GameOver mode.
 */
void
board_toggle_pause(Board *board)
{
	if (board->gameover)
		return;

	if (board->paused == true) {
		board->modal = false;
		board->paused = false;
		text_set_value(board->status_t, "");

		board_trash_temp_texts(board);
	} else {
		board->silent = false;
		board->modal = true;
		board->paused = true;
		if (board->silent == false)
			text_set_value(board->status_t, "paused!");
	}
}

/**
 * Apply a mask on top of all the rendered shit
 */
void
board_refresh_transition(Board *board)
{
	switch (board->transition) {
		case TTYPE_SHUTTER_OPEN:
			surface_shutter_open();
			board->transition = TTYPE_NONE;
			break;
		case TTYPE_PIXEL_OPEN:
			surface_pixel_open();
			board->transition = TTYPE_NONE;
		case TTYPE_NONE:
		default:
			break;
	}

}


/**
 * Main refresh function, actually dump pixels on the screen.
 */
void
board_refresh(Board *board)
{
	/* Redraw the sky. */
	a_sky_refresh(board);

	/* Redraw the background. */
	SDL_BlitSurface(board->bg, NULL, screen, NULL);

	/* Redraw each blocks. */
	board_refresh_blocks(board);

	/* Redraw the next block. */
	board_refresh_next(board);

	/* Redraw all the cubes. */
	board_refresh_cubes(board);

	/* Animations */
	a_chimneys_refresh(board);

	/* Draw texts elements (meant to replace OSD), and modal */
	board_refresh_texts(board);

	/* Apply the transition if any */
	board_refresh_transition(board);

	/* Dig up the back buffer. */
	SDL_Flip(screen);
}


/**
 * Called when the game has ended, return the appropriate MTYPE.
 */
enum mtype
board_gameover(Board *board)
{
	/* Success is only for tutorial mode. */
	if (board->success) {
		if (board->next_level) {
			r_free(conf->next_level);
			conf->next_level = r_strcp(board->next_level);
		}
		return MTYPE_GAMEOVER_WIN;
	}

	/* The player made a hiscore! */
	if (hiscore_check(board->score) == true) {
		conf->last_score = board->score;
		return MTYPE_GAMEOVER_HISCORE;
	}

	/* The player lost and didn't make a hiscore */
	return MTYPE_GAMEOVER_LOSE;
}


/**
 * This function handles all the elements at ticking point, if anything
 * pushed the board to a game over, let the caller know with an mtype.
 */
enum mtype
board_update(Board *board, uint32_t now)
{
	if (board->paused == true)
		return MTYPE_NOP;

	board->elapsed += TICK;
	if (board->elapsed > 1000) {
		board->elapsed = 0;
		board->time_limit--;
	}

	if (board->time_limit == 0) {
		board->gameover = true;
		return MTYPE_GAMEOVER_TIMEOUT;
	}

	board_update_blocks(board, now);
	board_update_cubes(board, now);
	board_update_water(board, now);

	/* Stop here, something in the 'update' caused a gameover */
	if (board->gameover == true)
		return board_gameover(board);

	/* We need a new line! */
	if (board->rising_speed > -1 && board->next_line <= now) {
		if (board->next_line != 1)
			board_add_line(board);
		board->next_line = now + board->rising_speed * 1000;
	}

	/* We were requested to launch the next block */
	if (board->launch_next == true) {
		board_launch_next_block(board);
		board->launch_next = false;
	}

	/* Animations */
	a_chimneys_update(board, now);
	a_sky_update(board, now);

	return MTYPE_NOP;
}


/*
 * Block related functions
 */


/**
 * Generate a random line of cube at the bottom and move everything up one
 * cube.
 */
void
board_add_line(Board *board)
{
	int i;
	Cube *cube;

	for (i = 0; i < (BOARD_WIDTH * BOARD_HEIGHT); i++) {
		cube = board->cubes[i];

		if (cube == NULL)
			continue;

		if (i < BOARD_WIDTH) {
			board->gameover = true;
			continue;
		}

		cube->y--;
		board->cubes[i - BOARD_WIDTH] = board->cubes[i];
	}

	board_prepopulate(board, 1);
}

void
board_add_block(Board *board, Block *block)
{
	board->block_count++;

	board->blocks = realloc(board->blocks,
			board->block_count * sizeof(Block *));

	board->blocks[board->block_count - 1] = block;
}


/**
 * This function will load a different type of block depending on the difficulty of the
 * current board. If we have a pending queue, use it instead of generating
 * randomly.
 */
void
board_load_next_block(Board *board)
{
	long int r;

	if (board->bqueue_len) {
		board->next_block = board->bqueue[board->bqueue_len - 1];
		board->bqueue[board->bqueue_len - 1] = NULL;
		board->bqueue_len--;
		return;
	}

	switch (board->difficulty) {
		/* Only one cube blocks */
		case DIFF_EASIEST:
			board->next_block = block_new_one(board->allow_dynamite);
			break;
		/* One and two cubes blocks */
		case DIFF_EASY:
			r = rand() % 2 + 7;
			board->next_block = block_new_of_type(r);
			break;
		/* One, two and three cubes blocks */
		case DIFF_MEDIUM:
			r = rand() % 4 + 7;
			board->next_block = block_new_of_type(r);
			break;
		/* All the block types */
		case DIFF_HARD:
		default:
			board->next_block = block_new_random();
			break;
		/* Only the tetrominoes */
		case DIFF_ULTRA:
			r = rand() % 7;
			board->next_block = block_new_of_type(r);
			break;
	}
}


/**
 * Take whatever block is currently loaded as next_block, add it to the
 * list of blocks for this board, place it at the right place and pick a
 * new next_block.
 */
void
board_launch_next_block(Board *board)
{
	Block *nb = board->next_block;

//	printf("LAUNCH! (remains=%d)\n", board->remains);

	/* If we had 0 remaining blocks... you lost. */
	if (board->remains == 0) {
		printf("RAN OUT OF BLOCKS!\n");
		board->gameover = true;
		return;
	}

	/* Load the new block */
	board_add_block(board, nb);
	board->current_block = nb;
	nb->x = (byte)(board->width / 2) - (byte)(nb->size / 2);
	nb->y = 0;

	/* Decrement the remaining blocks */
	if (board->remains > 0)
		board->remains--;

	/* If we are NOW at 0, you are on your last block */
	if (board->remains == 0) {
		board->next_block = NULL;
		return;
	}

	board_load_next_block(board);
}



void
board_change_next_block(Board *board)
{
	block_kill(board->next_block);
	board_load_next_block(board);
}


/**
 * Handle the graphic update of the blocks.
 */
void
board_refresh_blocks(Board *board)
{
	byte i;
	SDL_Rect r;
	Block *block;
	SDL_Surface *s;

	for (i = 0; i < board->block_count; i++) {
		block = board->blocks[i];
		if (block == NULL)
			continue;

		s = block_get_surface(block);
	//	fprintf(stderr, "refreshing block %d %p\n", i, s);
		block_get_rectangle(block, &r);

		r.x += board->offset_x;
		r.y += board->offset_y;

		SDL_BlitSurface(s, NULL, screen, &r);

		SDL_FreeSurface(s);
	}
}


/**
 * Handle the graphic update the of the top right "next" block.
 */
void
board_refresh_next(Board *board)
{
	SDL_Surface *s;
	SDL_Rect r;

	if (board->next_block != NULL) {
		s = block_get_surface(board->next_block);
		block_get_rectangle(board->next_block, &r);

		/* Increment of the position of the preview window. */
		r.x += board->offset_x + 10;
		r.y += 20;

		switch (board->next_block->type) {
			case BLOCK_TYPE_ONE:
				r.x += BSIZE / 2;
				r.y += BSIZE / 2;
				break;
			case BLOCK_TYPE_TWO:
				r.y -= BSIZE / 2;
				break;
			case BLOCK_TYPE_THREE:
				r.x -= BSIZE / 2;
				r.y -= BSIZE / 2;
		}

		SDL_BlitSurface(s, NULL, screen, &r);
		SDL_FreeSurface(s);
	}
}


/**
 * Transfer all the cubes from a block to the board. This is anticipating the
 * death of a block. Set the cube_count to 0 to avoid duplicate killing. Also
 * checks for special cubes and execute them.
 */
void
board_transfer_cubes(Board *board, Block *block)
{
	int x, y, i, j;
	byte *pos = block->positions[block->current_position];
	Cube *cube;

	for (y = 0; y < block->size; y++) {
		for (x = 0; x < block->size; x++) {
			i = (y + block->y) * board->width + (x + block->x);
			j = y * block->size + x;

			if (pos[j] < 1)
				continue;

			cube = block->cubes[pos[j] - 1];

			board->cubes[i] = cube;

			if (block->existing_cubes != true)
				board->cube_count++;

			if (cube != NULL) {
				cube->x = block->x + x;
				cube->y = block->y + y;
			}

			block->cubes[pos[j] - 1] = NULL;

			/* If one of the cube is a bomb, trigger it! */
			switch (cube->type) {
				case CTYPE_BOMB:
					board_cube_bomb(board, cube);
					break;
			}
		}
	}

	block->cube_count = 0;
}


void
board_kill_row(Board *board, int row)
{
	int i;

	for (i = board->width * row; i < board->width * (row + 1); i++)
		if (board->cubes[i] != NULL)
			board_trash_cube(board, board->cubes[i]);
}


void
board_kill_column(Board *board, int col)
{
	int i;

	for (i = col; i < board->width * board->height; i += board->width)
		if (board->cubes[i] != NULL)
			board_trash_cube(board, board->cubes[i]);
}


/**
 * A Cube Bomb just dropped! Execute its explosion and clear it up!
 */
void
board_cube_bomb(Board *board, Cube *cube)
{
	if (cube->current_position % 2 == 0)
		board_kill_row(board, cube->y);
	else
		board_kill_column(board, cube->x);

	sfx_play_boom();
}


/**
 * Loop through all the blocks and call the individual block update, skipping
 * NULL blocks (previously killed). It also handle the speeding up of the game
 * with increasing score.
 */
void
board_update_blocks(Board *board, uint32_t now)
{
	int i;

	if (board->score < 1000)
		board->block_speed = SPEED_NORMAL;
	else if (board->score < 5000)
		board->block_speed = SPEED_LESS5K;
	else if (board->score < 10000)
		board->block_speed = SPEED_LESS10K;
	else if (board->score < 25000)
		board->block_speed = SPEED_LESS25K;
	else if (board->score < 50000)
		board->block_speed = SPEED_LESS50K;
	else
		board->block_speed = SPEED_MAX;

	if (board->block_speed_factor > 1) {
		board->block_speed /= board->block_speed_factor;
	}

	for (i = 0; i < board->block_count; i++) {
		if (board->blocks[i] == NULL)
			continue;

		board_update_single_block(board, now, i);

		if (board->gameover == true)
			break;
	}
}


/**
 * Update an individual block during the current tick (which time is
 * reprensented by 'now'.
 */
void
board_update_single_block(Board *board, uint32_t now, int i) {
	Block *block = board->blocks[i];

	/* This block's tick has expired, we need to move it. */
	if (block->falling && now - block->tick > board->block_speed) {
		/* Can it fit one unit lower? */
		if (board_move_check(board, block, 0, 1) == 0) {
			block->y++;
			block->tick = now;
		}

		/* If the block didn't move, block it, and un-current */
		if (block->prev_y == block->y) {
			block->falling = false;
			if (block == board->current_block) {
				board->current_block = NULL;
				if (block->y > 0) {
					sfx_play_tack1();
					board->launch_next = true;
				} else {
					printf("NOT ENOUGH SPACE!\n");
					board->gameover = true;
					return;
				}
			}
			board_transfer_cubes(board, block);
			block_kill(block);
			board->blocks[i] = NULL;
			return;
		}

		/* Keep a reference to see if the block moved. */
		block->prev_y = block->y;
	}

	/* Ticking for the lateral moves. */
	if (now - board->lateral_tick > board->lateral_speed) {
		if (board->moving_left > 1) {
			board->moving_left--;
		} else if (board->moving_left == 1) {
			board_move_current_block_left(board);
		}

		if (board->moving_right > 1) {
			board->moving_right--;
		} else if (board->moving_right == 1) {
			board_move_current_block_right(board);
		}

		board->lateral_tick = now;
	}
}	


/* move_check() - 
 * 	bside is the value telling if the cube is on the left of the whole
 * 		block (1) or on the right side (2).
 *
 * Return values:
 * 	0 when path is clear
 * 	3 when touching the bottom
 * 	1 when a cube from the left side blocked
 * 	2 when a cube from the right side blocked */
byte
board_move_check(Board *board, Block *block, Sint8 x, Sint8 y)
{
	byte mx, my;
	byte bside;
	int i, j;
	byte *pos = block->positions[block->current_position];

	/* Update the map. */
//	board_update_map(board);

	/* For every cubes in this block, you need to check if there is a
	 * space in whatever direction. */
	for (my = 0; my < block->size; my++) {
		for (mx = 0; mx < block->size; mx++) {
			i = my * block->size + mx;
			bside = mx < block->size / 2 ? 1 : 2;

			/* Skip void cubes. */
			if (pos[i] == 0)
				continue;

			/* Reached the bottom of the board. */
			if (block->y + my + y >= board->height)
				return 3;

			/* Reached the left border. */
			if (block->x + mx + x < 0)
				return 1;

			/* Reach the right border. */
			if (block->x + mx + x >= board->width)
				return 2;
			
			/* There is a block in this direction. */
			j = (my + block->y + y) * board->width + (mx + block->x + x);
			if (board->cubes[j]) {
				return bside;
			}
		}
	}

	return 0;
}


void
board_move_current_block_left(Board *board)
{
	Block *block = board->current_block;

	if (block == NULL)
		return;

	/* Don't rotate cube during pause. */
	if (board->paused == true)
		return;

	if (board_move_check(board, block, -1, 0) == 0) {
		block->x--;
	}
//	board_dump_block_map(board);
}


void
board_move_current_block_right(Board *board)
{
	Block *block = board->current_block;

	if (block == NULL)
		return;

	/* Don't rotate cube during pause. */
	if (board->paused == true)
		return;

	if (board_move_check(board, block, 1, 0) == 0) {
		block->x++;
	}
}


/**
 * In charge of rotating a block on a board. If there is no space for a
 * rotation, ignore, if against a block, move a bit. If block is NULL,
 * default to current.
 */
void
board_rotate_cw(Board *board, Block *block)
{
	byte x;

	if (block == NULL)
		block = board->current_block;

	/* Don't rotate cube during pause. */
	if (board->paused == true)
		return;

	block_rotate_cw(block);

	/* Check if the new position is conflicting, if the conflict is on the
	 * left (1), x+1, if the conflict is on the right, x-1. */
	x = board_move_check(board, block, 0, 0);

	if (x == 1 && board_move_check(board, block, 1, 0) == 0) {
		block->x++;
		return;
	} else if (x == 2 && board_move_check(board, block, -1, 0) == 0) {
		block->x--;
		return;
	}
	
	/* If we had no issue rotating, we're good, play a sound ;) */
	if (x == 0) {
		sfx_play_tick1();
		return;
	}

	/* If we get that far, it's because we didn't manage to rotate the 
	 * block properly... back up. */
	block_rotate_ccw(block);
}


/*
 * Cube related functions.
 */


/**
 * Update all the cubes logic (non-graphic stuff).
 */
void
board_update_cubes(Board *board, uint32_t now)
{
	int i;
	int size = board->width * board->height;
	int type;
	Cube *cube;
	Block *block;

	for (i = 0; i < size; i++) {
		cube = board->cubes[i];

		if (cube == NULL)
			continue;
		
		/* Check if the cube is trashed (if yes fade it to death) */
		if (cube->trashed == true) {
			cube->fade_status++;
			if (cube->fade_status > BSIZE / 2) {
				cube_kill(cube);
				board->cubes[i] = NULL;
//				board->cube_count--;
			}
			continue;
		}

		/* If the cube is a rock, just skip, rocks don't move */
		if (cube->type == CTYPE_ROCK)
			continue;

		/* Check if the cube has free space under itself, if yes 
		 * disconnect it as a cube and create a new block. */
		type = board_get_area_type(board, cube->x, cube->y + 1);
		if (type == ATYPE_FREE) {
			block = block_new_one_from_cube(cube);
			block->existing_cubes = true;
			board_add_block(board, block);
			block->x = cube->x;
			block->y = cube->y;
			board->cubes[i] = NULL;
		}
	}

//	printf("update_cubes() count=%d blocks=%d\n", board->cube_count,
//			board->block_count);

	/* Check the cube count for CLEARALL levels. */
	if (board->objective_type == OBJTYPE_CLEARALL && 
			board->cube_count == 0) {
		board->gameover = true;
		board->success = true;
	}
}


/**
 * Refreshing the cubes is actually handling the graphic part, i.e. blitting
 * the textures at the right place.
 */
void
board_refresh_cubes(Board *board)
{
	int i;
	SDL_Rect r;
	Cube *cube;
	SDL_Surface *s;
	int size = board->width * board->height;

	for (i = 0; i < size; i++) {
		cube = board->cubes[i];
		if (cube == NULL) continue;

		s = cube_get_surface(cube);
		cube_get_rectangle(cube, &r);

		r.x += board->offset_x;
		r.y += board->offset_y;

		SDL_BlitSurface(s, NULL, screen, &r);
		SDL_FreeSurface(s);
	}
}


/**
 * Dump on stdout the map of the cube as it stands right now.
 */
void
board_dump_cube_map(Board *board)
{
	byte x, y;
	int i;

	for (y = 0; y < board->height; y++) {
		for (x = 0; x < board->width; x++) {
			i = y * board->width + x;
			if (board->cubes[i]) {
				printf("x");
			} else {
				printf(" ");
			}
		}
		printf("\n");
	}

}


/**
 * Go through all the cubes and remove the water, untie all the current
 * networks.
 */
void
board_remove_water(Board *board)
{
	int i;
	int bs = board->width * board->height;

	/* Get rid of all the water. */
	for (i = 0; i < bs; i++) {
		if (board->cubes[i] == NULL)
			continue;

		board->cubes[i]->water = 0;
		board->cubes[i]->root = NULL;
		board->cubes[i]->network_integrity = 1;
		cube_network_flush(board->cubes[i]);
	}
}


/**
 * Check the left and right side for cubes. If any, check if they have opened
 * pipes on the same side.
 */
void
board_update_water(Board *board, uint32_t now)
{
	int i;
	Cube *cube;

	board_remove_water(board);

	/* Scan the left side... */
	for (i = 0; i < board->height; i++) {
		cube = board->cubes[i * board->width];
		if (cube == NULL) continue;

		if (cube_plug_match(cube, PLUG_WEST))
			board_spread_water(board, cube, NULL, 1);
	}

	/* Now while scanning the right side, also check if water made it all
	 * the way through, in this case, taint the network. */
	for (i = 0; i < board->height; i++) {
		cube = board->cubes[(i + 1) * board->width - 1];
		if (cube == NULL) continue;

		if (cube_plug_match(cube, PLUG_EAST)) {
			if (cube->water == 1) {
				cube_network_taint(cube->root);
				/* If we are in link mode, this is a win */
				if (board->objective_type == OBJTYPE_LINK) {
					board->gameover = true;
					board->success = true;
				}
			} else
				board_spread_water(board, cube, NULL, 2);
		}
	}

	/* Now we know what networks are tainted, go through the left side
	 * again and look for a root cube (net length > 1), with red water (3),
	 * and a network_integrity preserved. Toggle an avalanche if found. */
	for (i = 0; i < board->height; i++) {
		cube = board->cubes[i * board->width];
		if (cube == NULL) continue;

		if (cube->network_size > 1 && cube->water == 3 &&
				cube->network_integrity == 1) {
			if (cube->fade_status > 0) continue;
			board_run_avalanche(board, cube);
		}
	}
}


/**
 * Run avalanche on the specific cube. This means the cube was already
 * identified as the root of a tainted network.
 */
void
board_run_avalanche(Board *board, Cube *cube)
{
	int i;
	Text *avtxt;

	/* Start a fading text... */
	avtxt = board_add_text(board, "Excellent!", 240, 200);
	avtxt->centered = true;
	avtxt->temp = true;
	text_set_color1(avtxt, 255, 0, 0);
	text_set_color2(avtxt, 80, 0, 0);
	avtxt->effect |= EFFECT_SHAKE|EFFECT_FADEOUT;

	board->score += 200;

	/* Run each columns individually */
	board_run_avalanche_column(board, cube);
	for (i = 0; i < cube->network_size; i++) {
		board_run_avalanche_column(board, cube->network[i]);
	}
}


/**
 * Run an avalanche on only one column, starting from the cube
 */
void
board_run_avalanche_column(Board *board, Cube *cube)
{
	int y;
	Cube *target;

	for (y = cube->y; y < board->height; y++) {
		target = board_get_cube(board, cube->x, y);
		/* Spare the rocks */
		if (target->type == CTYPE_ROCK)
			continue;
		if (target != NULL && target->trashed == false) {
			board_trash_cube(board, target);
			board->score += 20;
		}
	}
}


/**
 * Return a cube at the given coordinates, return NULL if not found or if
 * border of the board.
 */
Cube *
board_get_cube(Board *board, int x, int y)
{
	/* Bad x value */
	if (x < 0 || x >= board->width)
		return NULL;

	/* Bad y value */
	if (y < 0 || y >= board->height)
		return NULL;

	return (board->cubes[x + board->width * y]);
}


/**
 * Return a cube at the given screen coordinates, return NULL if nothing
 * found.
 */
Cube *
board_get_cube_absolute(Board *board, int x, int y)
{
	int rx, ry;

	rx = (x - BOARD_LEFT) / BSIZE;
	ry = (y - BOARD_TOP) / BSIZE;

	return board_get_cube(board, rx, ry);
}


Block *
board_get_block(Board *board, int x, int y)
{
	Block *b;
	int i;

	/* Bad x value */
	if (x < 0 || x >= board->width)
		return NULL;

	/* Bad y value */
	if (y < 0 || y >= board->height)
		return NULL;

	/* Loop through all the currently active blocks to find out
	 * if one matchs the current call. */
	for (i = 0; i < board->block_count; i++) {
		b = board->blocks[i];
		
		if (b == NULL)
			continue;

		if (b->x == x && b->y == y)
			return b;
	}

	return NULL;
}


/**
 * Returns a block or NULL given screen coordinates.
 */
Block *
board_get_block_absolute(Board *board, int x, int y)
{
	int rx, ry;

	rx = (x - BOARD_LEFT) / BSIZE;
	ry = (y - BOARD_TOP) / BSIZE;

	return board_get_block(board, rx, ry);
}


/**
 * Returns an integer representing the type of area at x,y
 */
int
board_get_area_type(Board *board, int x, int y)
{
	if (x < 0)
		return ATYPE_BOARD_LEFT;

	if (x >= board->width)
		return ATYPE_BOARD_RIGHT;

	if (y >= board->height)
		return ATYPE_BOARD_BOTTOM;

	if (board->cubes[x + board->width * y] != NULL)
		return ATYPE_BLOCK;

	return ATYPE_FREE;
}


/**
 * Given specific x/y offsets, try to spread the water to a neighboring cube
 */
void
board_spread_attempt(Board *board, Cube *cube, Cube *root, Sint8 ox, Sint8 oy,
		byte src_plug, byte dest_plug)
{
	Cube *n;
	int status;
	int type;

	type = board_get_area_type(board, cube->x + ox, cube->y + oy);
	switch (type) {
		case ATYPE_FREE:
			if (cube_plug_match(cube, src_plug))
				root->network_integrity = 0;
			break;
		case ATYPE_BLOCK:
			n = board_get_cube(board, cube->x + ox, cube->y + oy);
			status = cube_get_plug_status(cube, src_plug, n, 
					dest_plug);
			if (status == PSTAT_CONNECTED)
				board_spread_water(board, n, root, root->water);
			break;
		default:
			break;
	}
}


/**
 * A network is going to be destroyed.
 */
void
board_destroy_network(Board *board, Cube *cube)
{
	int i;

	if (cube->trashed == true)
		return;

	for (i = 0; i < cube->network_size; i++)
		board_trash_cube(board, cube->network[i]);

	board_trash_cube(board, cube);
	board->score += (cube->network_size + 1) * 8;

	sfx_play_lazer();
}


/**
 * Mark a cube for deletion (trashed), it will fade and ultimately be removed
 */
void
board_trash_cube(Board *board, Cube *cube)
{
	cube->trashed = true;
	board->cube_count--;
}


/**
 * Take a cube and check its neighbors for propagation. 'n' is always the
 * neighbor cube.
 */
void
board_spread_water(Board *board, Cube *cube, Cube *root, int water_type)
{
	/* Don't continue if this cube is already watered. */
	if (cube->water >= 1)
		return;

	/* Add 'cube' to the network, it doesn't seem to be a starting point. */
	if (root == NULL) {
		root = cube;
	} else {
		cube_network_add(root, cube);
	}

	cube->water = water_type;

	/* North, East, South, West */
	board_spread_attempt(board, cube, root,  0, -1, PLUG_NORTH, PLUG_SOUTH);
	board_spread_attempt(board, cube, root,  1,  0, PLUG_EAST,  PLUG_WEST);
	board_spread_attempt(board, cube, root,  0,  1, PLUG_SOUTH, PLUG_NORTH);
	board_spread_attempt(board, cube, root, -1,  0, PLUG_WEST,  PLUG_EAST);

	if (root == cube) {
		if (cube->network_integrity == 1) {
			board_destroy_network(board, cube);
		}
	}
}


/**
 * Prepopulate a number of lines at the bottom of the board.
 * (Keywords: generate, line, fill)
 */
void
board_prepopulate(Board *board, int lines)
{
	int x, y;
	Cube *cube;

	for (x = 0; x < board->width; x++) {
		for (y = board->height - lines; y < board->height; y++) {
			cube = cube_new_random();
			cube->x = x;
			cube->y = y;
			board->cubes[y * board->width + x] = cube;
			board->cube_count++;
		}
	}
}

