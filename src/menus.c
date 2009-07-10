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
#include <time.h>

#include "SDL.h"

#include "rezerwar.h"

enum toggle_types {
	TOGGLE_DIFFICULTY,
	TOGGLE_SOUND,
	TOGGLE_FULLSCREEN
};

extern Configuration *conf;
extern SDL_Surface *screen;
extern SDL_Surface *sprites;

/* This is a dump of the screen before the menu is drawn. */
SDL_Surface *gameover_background;


typedef struct _menu_item {
	Text *text;
	SDL_Rect rect;
	int type;
	int subtype;
	int x_offset;
} MenuItem;

typedef struct _menu {
	int x;
	int y;
	int cursor_x;
	int cursor_y;
	char *bg_image;
	SDL_Surface *bg_surface;
	bool modal;
	bool bg_refresh;
	MenuItem **items;
	int length;
	int current;
} Menu;


void menuitem_kill(MenuItem *);


/**
 * Menus are keeping track of the current selected item and a list of all
 * their items.
 */
Menu *
new_menu(void)
{
	Menu *menu;

	menu = r_malloc(sizeof(Menu));
	menu->items = NULL;
	menu->length = 0;
	menu->current = 0;
	menu->bg_image = NULL;
	menu->bg_refresh = true;
	menu->bg_surface = NULL;
	menu->modal = false;

	return menu;
}


/**
 * Clean up all the items and reset everything.
 */
void
flush_menu_items(Menu *menu)
{
	int i;

	if (menu->items == NULL)
		return;

	for (i = 0; i < menu->length; i++) {
		menuitem_kill(menu->items[i]);
	}
	free(menu->items);
	menu->items = NULL;
	menu->length = 0;
	menu->current = 0;
}



/**
 * Destructor for the menu.
 */
void
menu_kill(Menu *menu)
{
	flush_menu_items(menu);
	r_free(menu->bg_image);
	r_free(menu);
}


void
menuitem_kill(MenuItem *item)
{
	text_kill(item->text);
	r_free(item);
}



void
menu_item_set_difficulty(MenuItem *item)
{
	char *diff_t[] = {
		"Difficulty: Easiest",
		"Difficulty: Easy",
		"Difficulty: Medium",
		"Difficulty: Hard",
		"Difficulty: Ultra" };

	text_set_value(item->text, diff_t[conf->difficulty]);
}

void
menu_item_set_sound(MenuItem *item)
{
	char *sound_t[] = {
		"Sound/Music: Off",
		"Sound/Music: On" };

	text_set_value(item->text, sound_t[conf->sound]);
}

void
menu_item_set_fullscreen(MenuItem *item)
{
	char *fullscreen_t[] = {
		"Fullscreen: Off",
		"Fullscreen: On" };

	text_set_value(item->text, fullscreen_t[conf->fullscreen]);
}

/**
 * This function creates a new entry in the menu, it assumes you know
 * what you are doing and all the values are valid.
 */
MenuItem *
add_item_to_menu(Menu *menu, char *text, int type, int subtype, int x_offset)
{
	MenuItem *item;

	item = r_malloc(sizeof(MenuItem));
	item->type = type;
	item->subtype = subtype;
	item->x_offset = x_offset;
	item->rect.x = 0;
	item->rect.y = 0;
	item->rect.w = 0;
	item->rect.h = 0;

	item->text = text_new(text);
	item->text->centered = true;
	text_set_color2(item->text, 0x00, 0x0e, 0x26);
	text_set_color1(item->text, 0x58, 0x89, 0xc6);

	menu->length++;
	menu->items = realloc(menu->items, sizeof(MenuItem*) * menu->length);
	menu->items[menu->length - 1] = item;

	return item;
}


void
menu_load_main(Menu *menu)
{
	flush_menu_items(menu);
	add_item_to_menu(menu, "tutorial", MTYPE_START, 0, 0);
	add_item_to_menu(menu, "play", MTYPE_PLAIN, 0, 0);
	add_item_to_menu(menu, "options", MTYPE_SUBMENU, 2, 45);
	add_item_to_menu(menu, "quit", MTYPE_QUIT, 0, 65);
}


void
menu_load_gameover(Menu *menu, bool allow_next_level)
{
	flush_menu_items(menu);
	if (allow_next_level == true)
		add_item_to_menu(menu, "next level", MTYPE_NEXTLEVEL, 0, 0);
	add_item_to_menu(menu, "replay level", MTYPE_REPLAY, 0, 0);
	add_item_to_menu(menu, "main menu", MTYPE_BREAK, 1, 45);
	add_item_to_menu(menu, "quit rzwar", MTYPE_QUIT, 0, 65);
}


/**
 * For this menu we keep a reference to the difficulty menu item so we can load its
 * value/name properly after.
 */
void
menu_load_options(Menu *menu)
{
	MenuItem /* *diff_item,*/ *sound_item, *fs_item;

	flush_menu_items(menu);

	/*
	diff_item = add_item_to_menu(menu, "difficulty", MTYPE_TOGGLE, 
			TOGGLE_DIFFICULTY, 0);
	menu_item_set_difficulty(diff_item);
	*/

	sound_item = add_item_to_menu(menu, "sound/music", MTYPE_TOGGLE,
			TOGGLE_SOUND, 0);
	menu_item_set_sound(sound_item);

#ifndef __WII__
	fs_item = add_item_to_menu(menu, "fullscreen", MTYPE_TOGGLE,
			TOGGLE_FULLSCREEN, 0);
	menu_item_set_fullscreen(fs_item);
#endif

	menu_item_set_sound(sound_item);
	add_item_to_menu(menu, "back to main", MTYPE_SUBMENU, 0, 0);
}


void
menu_load_submenu(Menu *menu, int subtype)
{
	switch (subtype) {
		case 0: // main menu no refresh
			menu_load_main(menu);
			break;
		case 2: // options menu
			menu_load_options(menu);
			break;
		default:
			break;
	}
}


void
menu_toggle_item(Menu *menu, MenuItem *item)
{
	switch (item->subtype) {
		/* Difficulty */
		case TOGGLE_DIFFICULTY:
			conf->difficulty++;
			if (conf->difficulty >= DIFF_LENGTH)
				conf->difficulty = 0;
			menu_item_set_difficulty(item);
			break;
		case TOGGLE_FULLSCREEN:
			conf->fullscreen = !conf->fullscreen;
#ifdef _WIN32
			init_gfx();
#else
			if (SDL_WM_ToggleFullScreen(screen) == 0)
				fatal("Unable to toggle fullscreen/windowed mode.");
#endif
			menu_item_set_fullscreen(item);
			break;
		case TOGGLE_SOUND:
			sfx_toggle_mute(conf->sound);
			conf->sound = !conf->sound;
			menu_item_set_sound(item);
			break;
		default:
			break;
	}
}


/**
 * Execute whatever is corresponding to the type of the current entry.
 */
enum mtype
menu_select(Menu *menu)
{
	MenuItem *item = menu->items[menu->current];

	sfx_play_menuselect();

	switch (item->type) {
		case MTYPE_SUBMENU:
			menu_load_submenu(menu, item->subtype);
			return 0;
		case MTYPE_TOGGLE:
			menu_toggle_item(menu, item);
			return 0;
		default:
			break;
	}

	return item->type;
}




void
menu_refresh(Menu *menu)
{
	int i;
	MenuItem *item;
	SDL_Surface *s;

	for (i = 0; i < menu->length; i++) {
		item = menu->items[i];

		item->text->x = menu->x + item->x_offset;
		item->text->y = menu->y + i * 30;
		if (i == menu->current) {
			item->text->colorized = true;
//			item->text->effect |= EFFECT_SHAKE;
			item->text->effect |= EFFECT_WAVE;
		} else {
			item->text->colorized = false;
//			item->text->effect &= ~EFFECT_SHAKE;
			item->text->effect &= ~EFFECT_WAVE;
		}
		s = text_get_surface(item->text);
		if (s == NULL)
			continue;

		text_get_rectangle(item->text, &(item->rect));
		SDL_BlitSurface(s, NULL, screen, &(item->rect));
		SDL_FreeSurface(s);
	}

//	blit_cursor(0, menu->cursor_x, menu->cursor_y);
}

bool
hover_menu_items(MenuItem *item, SDL_MouseButtonEvent *bev) 
{
	if (bev->x > item->rect.x &&
			bev->x < (item->rect.x + item->rect.w) &&
			bev->y > item->rect.y &&
			bev->y < (item->rect.y + item->rect.h)) {
		return true;
	}
	return false;
}

int
handle_menu_events(SDL_Event *event, Menu *menu)
{
	SDL_MouseButtonEvent *bev;
	enum { up, down, select, escape, nothing } action = nothing;
	int i;

	if (event->type == SDL_QUIT)
		exit(0);

	if (event->type == SDL_MOUSEBUTTONDOWN) {
		bev = &event->button;
		for (i = 0; i < menu->length; i++) {
			if (hover_menu_items(menu->items[i], bev) == true) {
				return menu_select(menu);
			}
		}
		return 0;
	}

	if (event->type == SDL_MOUSEMOTION) {
		bev = &event->button;
		menu->cursor_x = bev->x;
		menu->cursor_y = bev->y;
		for (i = 0; i < menu->length; i++) {
			if (menu->current == i)
				continue;
			if (hover_menu_items(menu->items[i], bev) == true) {
				menu->current = i;
				sfx_play_menunav();
			}
		}
		return 0;
	}

	if (event->type == SDL_JOYHATMOTION) {
		if (event->jhat.value & SDL_HAT_UP)
			action = up;
		else if (event->jhat.value & SDL_HAT_DOWN)
			action = down;
	}

#ifdef __WII__
	if (event->type == SDL_JOYBUTTONDOWN) {
		if (event->jbutton.button == WPAD_BUTTON_1 || 
		    event->jbutton.button == WPAD_BUTTON_2) {
			action = select;
		}
	}
#endif
	

	/* Handle keyboard input */
	if (event->type == SDL_KEYDOWN) {
		switch ((int)event->key.keysym.sym) {
			case SDLK_j:
			case SDLK_DOWN:
				action = down;
				break;
			case SDLK_UP:
			case SDLK_k:
				action = up;
				break;
			case SDLK_RETURN:
				action = select;
				break;
			case SDLK_ESCAPE:
			case SDLK_q:
				action = escape;
				break;
			default:
				break;
		}
	}

	/* Act on whatever input was caught */
	switch (action) {
		case down:
			menu->current++;
			sfx_play_menunav();
			if (menu->current >= menu->length)
				menu->current = 0;
			break;
		case up:
			menu->current--;
			sfx_play_menunav();
			if (menu->current < 0)
				menu->current = menu->length - 1;
			break;
		case select:
			return menu_select(menu);
		case escape:
			return 1;
		default:
			break;
	}

	return 0;
}


enum mtype
menu_runner(Menu *menu)
{
	SDL_Surface *intro = NULL;
	uint32_t now, framecount = 0, fps_lastframe = 0;
	byte running = 0;
	int elapsed;
	SDL_Event event;

	while (running == 0) {
		while (SDL_PollEvent(&event)) {
			running = handle_menu_events(&event, menu);
		}

		if (menu->bg_refresh == true) {
			/* Load the initial image and fade into it. */
			if (menu->bg_image != NULL) {
				intro = SDL_LoadBMP(menu->bg_image);
				surface_fadein(intro, 16);
			} else if (menu->bg_surface != NULL) {
				SDL_BlitSurface(menu->bg_surface, NULL,
						intro, NULL);
			} else {
				intro = copy_screen();
			}
			menu->bg_refresh = false;
		}

		now = SDL_GetTicks();

		/* Every 1.0 / MAXFPS seconds, refresh the screen. */
		if (fps_lastframe < (now - (1000/MAXFPS))) {
			framecount++;
			fps_lastframe = now;
			SDL_BlitSurface(intro, NULL, screen, NULL);

			if (menu->modal == true)
				blit_modal(160);

			menu_refresh(menu);
			SDL_Flip(screen);
		}

		elapsed = SDL_GetTicks() - now;
		if (elapsed < TICK) {
			SDL_Delay(TICK - elapsed);
		}
	}

	SDL_FreeSurface(intro);

	return running;
}


int
main_menu()
{
	Menu *menu;
	int status;

	menu = new_menu();
	menu->x = 224;
	menu->y = 245;
	menu->bg_image = dpath("gfx/gamemenu.bmp");

	menu_load_main(menu);

	sfx_play_music("music/menu.mp3");
//	sfx_play_music("music/menu.ogg");
	status = menu_runner(menu);

	menu_kill(menu);

	return status;
}


int
gameover_menu(enum mtype status)
{
	Menu *menu;
	Text *title;
	bool allow_next_level = true;

	if (conf->next_level == NULL || status != MTYPE_GAMEOVER_WIN)
		allow_next_level = false;

	/* Dump a modal and a title before running the menu */
	blit_modal(160);
	switch (status) {
		case MTYPE_GAMEOVER_WIN:
			title = text_new("Congratulations!");
			break;
		case MTYPE_GAMEOVER_TIMEOUT:
			title = text_new("Time out!");
			break;
		default:
		case MTYPE_GAMEOVER_LOSE:
			title = text_new("You failed!");
			break;
	}

	title->centered = true;
	title->y = 200;
	text_set_colors(title, 0xff9020, 0xa9440d);
	text_blit(title, screen);

	/* Actually run the menu */
	menu = new_menu();
	menu->x = 200;
	menu->y = 285;
	menu_load_gameover(menu, allow_next_level);
	status = menu_runner(menu);

	menu_kill(menu);

	SDL_FreeSurface(gameover_background);

	return status;
}
