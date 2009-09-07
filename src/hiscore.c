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
#include <stdlib.h>
#include <time.h>

#include "SDL.h"

#include "rezerwar.h"

extern Configuration *conf;
extern SDL_Surface *screen;
HiScore **hs = NULL;

/**
 * Called from most function to load the hiscore table in case it isn't already
 * in memory.
 */
void
hiscore_load()
{
	int i;
	size_t len;
	FILE *fp;
	char *buffer;
	char *c;
	char *path;

	if (hs == NULL) {
		hs = malloc(sizeof(HiScore*) * 10);

		for (i = 0; i < 10; i++) {
			hs[i] = malloc(sizeof(HiScore));
			strlcpy(hs[i]->name, "Anonymous", 16);
			hs[i]->score = 0;
			hs[i]->date = 0;
		}

		path = cpath("hiscore.dat");
		fp = fopen(path, "r");
		r_free(path);
		if (fp == NULL)
			return;

		buffer = malloc(512);
		len = fread(buffer, 1, 512, fp);
		buffer[len] = 0;
		fclose(fp);

		i = 0;
		while ((c = strsep(&buffer, "\n")) != NULL && i < 10) {
			sscanf(c, "%d:%d:%s", &(hs[i]->score),
					(int *)&(hs[i]->date), hs[i]->name);
			i++;
		}
		free(buffer);
	}
}


/**
 * In prompt mode, capture all the characters from the keyboard until
 * return. Return true when return was entered.
 */
bool
handle_events_prompt(SDL_keysym keysym, Text *text)
{
	char ch;

	if ((keysym.unicode & 0xFF80) != 0)
		return false;

	if (keysym.sym == SDLK_BACKSPACE) {
		text_del_last_char(text);
		return false;
	}

	if (keysym.sym == SDLK_RETURN) {
		return true;
	}

	ch = keysym.unicode & 0x7F;
	if (isalnum(ch) != 0)
		text_add_char(text, ch);

	return false;
}


bool
prompt_polling(Text *prompt)
{
	SDL_Event event;
	bool done = false;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_JOYBUTTONDOWN) {
			text_set_value(prompt, "wiiuser");
			done = true;
		}
		if (event.type != SDL_KEYDOWN)
			continue;
		done = handle_events_prompt(event.key.keysym, prompt);
	}

	return done;
}


enum mtype
hiscore_prompt() {
	SDL_Surface *back = copy_screen();
	Text *title, *name, *prompt;
	bool done = false;

	title = text_new("new high score!");
	title->centered = true;
	title->y = 200;
	title->font = 1;
	text_set_colors(title, 0xff9020, 0xa9440d);

	name = text_new(" enter your name:");
	name->centered = true;
	name->font = 1;
	name->y = 230;
	text_set_colors(name, 0xff9020, 0xa9440d);

	prompt = text_new("");
	prompt->centered = true;
	prompt->max_length = 20;
	prompt->y = 260;
	prompt->font = 1;
	text_set_colors(prompt, 0x6782cf, 0x010193);

	while (done == false) {
		done = prompt_polling(prompt);
		
		SDL_BlitSurface(back, NULL, screen, NULL);
		blit_modal(160);
		text_blit(title, screen);
		text_blit(name, screen);
		text_blit(prompt, screen);
		SDL_Flip(screen);
		SDL_Delay(50);
	}

	/* Restore the surface just in case ... */
	SDL_BlitSurface(back, NULL, screen, NULL);
	blit_modal(160);

	/* Record the score */
	if (prompt->length > 0)
		hiscore_add(prompt->value, conf->last_score);

	SDL_FreeSurface(back);
	text_kill(title);
	text_kill(name);
	text_kill(prompt);

	return MTYPE_HISCORES;
}

/**
 * Dump all the text on the board for the high score.
 */
void
hiscore_show()
{
	int i;
	Text *text;
	char buf[16];

	hiscore_load();
	text = text_new("");

	for (i = 0; i < 10; i++) {
		snprintf(buf, 16, "%d", hs[i]->score);
		text_set_value(text, buf);
		text->x = 200;
		text->y = 120 + 24 * i;
		text_set_color1(text, 80, 190, 100);
		text_set_color2(text, 30, 130, 40);
		text_blit(text, screen);

		text_set_value(text, hs[i]->name);
		text->x = 350;
		text_set_color1(text, 80, 100, 190);
		text_set_color2(text, 30, 40, 130);
		text_blit(text, screen);
	}

	text_kill(text);

	SDL_Flip(screen);

	wait_for_keymouse();
}

/**
 * Commit to file the current hiscore table.
 */
void
hiscore_save()
{
	int i;
	FILE *fp;
	char *path;

	path = cpath("hiscore.dat");
	fp = fopen(path, "w");
	r_free(path);
	if (fp == NULL)
		return;

	for (i = 0; i < 10; i++) {
		fprintf(fp, "%d:%u:%s\n", hs[i]->score, 
				(unsigned int)hs[i]->date, hs[i]->name);
	}

	fclose(fp);
}

/**
 * Add a new high score!
 */
void
hiscore_add(char *name, int score)
{
	int i, x;

	hiscore_load();

	/* At the end of this loop, i is the index of the item to replace */
	for (i = 0; i < 10; i++)
		if (score > hs[i]->score)
			break;
	x = i;
	
	/* Drop the last item and shift all of them. */
	r_free(hs[9]);
	for (i = 8; i > x; i--)
		hs[i] = hs[i - 1];

	hs[x] = malloc(sizeof(HiScore));
	strlcpy(hs[x]->name, name, 16);
	hs[x]->score = score;
	hs[x]->date = time(NULL);

	hiscore_save();
}

/**
 * Return whether the score is a hiscore or not, it assumes 'hs' is always
 * sorted from the highest to the lowest score.
 */
bool
hiscore_check(int score)
{
	int i;

	hiscore_load();

	for (i = 0; i < 10; i++) {
		if (score > hs[i]->score) {
			return true;
		}
	}

	return false;
}

/**
 * Just sanity function... called from main.
 */
void
hiscore_free()
{
	r_free(hs);
}

