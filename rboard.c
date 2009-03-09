#include <stdio.h>

#include "SDL.h"

#include "rezerwar.h"


extern Board *board;
extern SDL_Surface *screen;
extern Uint32 key;


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

	/* Cube related members initialization. */
	b->cubes = r_malloc(size * sizeof(Cube *));
	for (i = 0; i < size; i++)
		b->cubes[i] = NULL;

	/* Block related members. */
	b->block_count = 0;
	b->blocks = NULL;
	b->current_block = NULL;
	b->next_block = NULL;
	b->block_speed = SPEED_NORMAL;
	b->block_speed_factor = 1;

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
	b->paused = 0;
	b->gameover = 0;

	/* Prompt init. */
	b->prompt_text = NULL;
	b->prompt_func = NULL;

	/* Modal and score */
	b->modal = false;
	b->score_t = board_add_text(b, (unsigned char *)"0", 10, 10);

	/* Status message */
	b->status_t = board_add_text(b, (unsigned char *)"", 260, 240);
	b->status_t->effect = EFFECT_SHAKE;
	b->status_t->centered = true;
	text_set_color1(b->status_t, 225, 186, 0);
	text_set_color2(b->status_t, 127,  55, 0);

	/* (optional) FPS display */
	b->show_fps = false;
	b->fps_t = board_add_text(b, (byte *)"", 550, 10);
	text_set_color1(b->status_t, 225, 40, 0);
	text_set_color2(b->status_t, 56,  8, 8);

	/* Load background. */
	b->bg = SDL_LoadBMP("gfx/gameback.bmp");
	printf("Key: %u\n", key);
	SDL_SetColorKey(b->bg, SDL_SRCCOLORKEY|SDL_RLEACCEL, key);

	return b;
}

/**
 * Create a new board and populate the cubes from the level.
 */
Board *
board_new_from_level(Level *level)
{
	int i;
	Board *board;
	Cube *cube = NULL;

	board = board_new(0);

	for (i = 0; i < (BOARD_WIDTH * BOARD_HEIGHT); i++) {
		switch (level->cmap[i]) {
			case '+': cube = cube_new_type(0, CTYPE_ALL); break;
			case '-': cube = cube_new_type(0, CTYPE_FLAT); break;
			case '|': cube = cube_new_type(1, CTYPE_FLAT); break;
			case 'J': cube = cube_new_type(0, CTYPE_ANGLE); break;
			case 'L': cube = cube_new_type(1, CTYPE_ANGLE); break;
			case 'F': cube = cube_new_type(2, CTYPE_ANGLE); break;
			case '7': cube = cube_new_type(3, CTYPE_ANGLE); break;
			case '>': cube = cube_new_type(0, CTYPE_KNOB); break;
			case '_': cube = cube_new_type(1, CTYPE_KNOB); break;
			case '<': cube = cube_new_type(2, CTYPE_KNOB); break;
			case '^': cube = cube_new_type(3, CTYPE_KNOB); break;
			case 'A': cube = cube_new_type(0, CTYPE_TEE); break;
			case '}': cube = cube_new_type(1, CTYPE_TEE); break;
			case 'T': cube = cube_new_type(2, CTYPE_TEE); break;
			case '{': cube = cube_new_type(3, CTYPE_TEE); break;
			case '.':
			default:
				continue;
				break;
		}

		cube->y = i / BOARD_WIDTH;
		cube->x = i % BOARD_WIDTH;
		board->cubes[i] = cube;
	}

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

	/* General board clean up */
	SDL_FreeSurface(board->bg);
	r_free(board);
}


Text *
board_add_text(Board *board, unsigned char *value, int x, int y)
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
	SDL_Surface *s, *m;

	/* Update the score Text */
	unsigned char score[20];
	snprintf((char *)score, 20, "score: %d", board->score);
	text_set_value(board->score_t, score);

	/* Add a modal under all the text. */
	if (board->modal == true) {
		m = SDL_CreateRGBSurface(0, screen->w, screen->h, 
				screen->format->BitsPerPixel, 0, 0, 0, 0);
		SDL_FillRect(m, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
		SDL_SetAlpha(m, SDL_SRCALPHA|SDL_RLEACCEL, 160);
		SDL_BlitSurface(m, NULL, screen, NULL);
		SDL_FreeSurface(m);
	}

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
		text_get_rectangle(t, &r);

		SDL_BlitSurface(s, NULL, screen, &r);
		SDL_FreeSurface(s);
	}
}


void
board_toggle_pause(Board *board)
{
	if (board->paused == 1) {
		board->modal = false;
		board->paused = 0;
		text_set_value(board->status_t, (unsigned char *)"");
	} else {
		board->modal = true;
		board->paused = 1;
		text_set_value(board->status_t, (unsigned char *)"paused!");
	}
}


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

	/* Dig up the back buffer. */
	SDL_Flip(screen);
}


int
board_save_score(Text *text, Text *x)
{
	board->modal = true;
	if (text->length > 0) {
		hiscore_add((char *)text->value, board->score);
	}
	board->status_t->y = 80;
	text_set_value(board->status_t, (byte*)"High Scores");
	hiscore_dump(board);
	text->trashed = true;
	x->trashed = true;
	board->prompt_text = NULL;

	return 1;
}


/**
 * Called when the game has ended, if the score is a high one, prompt
 * for the name of the player.
 */
void
board_gameover(Board *board)
{
	Text *txt, *prompt;

	board->modal = true;
	board->gameover = 1;
	text_set_value(board->status_t, (unsigned char *)"game over!");
	if (hiscore_check(board->score) == false)
		return;

	txt = board_add_text(board, (byte *)"enter your name:", 0, 270);
	txt->centered = true;
	prompt = board_add_text(board, (byte *)"", 0, 310);
	text_set_color1(prompt, 80, 100, 190);
	text_set_color2(prompt, 30, 40, 130);
	prompt->centered = true;
	board->prompt_text = prompt;
	board->prompt_func = board_save_score;
	board->prompt_data = txt;
}


/* board_update() - this function handles all the elements at ticking point */
void
board_update(Board *board, u_int32_t now)
{
	if (board->paused == 1 || board->gameover == 1)
		return;

	board_update_blocks(board, now);
	board_update_cubes(board, now);
	board_update_water(board, now);

	/* Animations */
	a_chimneys_update(board, now);
	a_sky_update(board, now);
}

