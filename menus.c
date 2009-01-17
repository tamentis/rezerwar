#include <stdio.h>
#include <time.h>
#include <string.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


extern Configuration *conf;
extern SDL_Surface *screen;
extern SDL_Surface *sprites;


typedef struct _menu_item {
	char *text;
	int type;
	int subtype;
	int x_offset;
} MenuItem;

typedef struct _menu {
	int x;
	int y;
	MenuItem **items;
	int length;
	int current;
} Menu;



Menu *
menu_new(void)
{
	Menu *menu;

	menu = r_malloc(sizeof(Menu));
	menu->items = NULL;
	menu->length = 0;
	menu->current = 0;

	return menu;
}


void
menuitem_kill(MenuItem *item)
{
	r_free(item->text);
	r_free(item);
}



void
menu_flush(Menu *menu)
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



void
menu_kill(Menu *menu)
{
	menu_flush(menu);
	r_free(menu);
}


void
menu_item_set_text(MenuItem *item, char *text)
{
	size_t i = strlen(text);

	r_free(item->text);
	item->text = r_malloc(i + 1);
	strlcpy(item->text, text, i + 1);
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

	menu_item_set_text(item, diff_t[conf->difficulty]);
}


/**
 * This function creates a new entry in the menu, it assumes you know
 * what you are doing and all the values are valid.
 */
MenuItem *
menu_add_entry(Menu *menu, char *text, int type, int subtype, int x_offset)
{
	MenuItem *item;

	item = r_malloc(sizeof(MenuItem));
	item->text = NULL;
	menu_item_set_text(item, text);
	item->type = type;
	item->subtype = subtype;
	item->x_offset = x_offset;

	menu->length++;
	menu->items = realloc(menu->items, sizeof(MenuItem*) * menu->length);
	menu->items[menu->length - 1] = item;

	return item;
}


void
menu_load_main(Menu *menu)
{
	menu_flush(menu);
	menu_add_entry(menu, "start new game", MTYPE_START, 0, 0);
	menu_add_entry(menu, "options", MTYPE_SUBMENU, 2, 45);
	menu_add_entry(menu, "quit", MTYPE_QUIT, 0, 65);
}


/**
 * For this menu we keep a reference to the difficulty menu item so we can load its
 * value/name properly after.
 */
void
menu_load_options(Menu *menu)
{
	MenuItem *diff_item;

	menu_flush(menu);
	diff_item = menu_add_entry(menu, "difficulty", MTYPE_TOGGLE, 0, 0);
	menu_add_entry(menu, "things", MTYPE_NOP, 0, 0);
	menu_add_entry(menu, "back to main", MTYPE_SUBMENU, 0, 0);

	menu_item_set_difficulty(diff_item);
}


void
menu_load_submenu(Menu *menu, int subtype)
{
	switch (subtype) {
		case 0:
			menu_load_main(menu);
			break;
		case 2:
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
		case 0:
			conf->difficulty++;
			if (conf->difficulty >= DIFF_LENGTH)
				conf->difficulty = 0;
			menu_item_set_difficulty(item);
			break;
		default:
			break;
	}
}


/**
 * Execute whatever is corresponding to the type of the current entry.
 */
int
menu_select(Menu *menu)
{
	MenuItem *item = menu->items[menu->current];

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

	for (i = 0; i < menu->length; i++) {
		item = menu->items[i];

		if (i == menu->current) {
			osd_print_moving(item->text, menu->x + item->x_offset,
					menu->y + i * 30, 2);
		} else {
			osd_print(item->text, menu->x + item->x_offset, 
					menu->y + i * 30);
		}
	}
}


int
handle_menu_events(SDL_Event *event, Menu *menu)
{
	if (event->type != SDL_KEYDOWN)
		return 0;

	switch ((int)event->key.keysym.sym) {
		case SDLK_j:
		case SDLK_DOWN:
			menu->current++;
			if (menu->current >= menu->length)
				menu->current = 0;
			break;
		case SDLK_UP:
		case SDLK_k:
			menu->current--;
			if (menu->current < 0) {
				menu->current = menu->length - 1;
			}
			break;
		case SDLK_RETURN:
			return menu_select(menu);
		case SDLK_ESCAPE:
			return 1;
		default:
			break;

	}

	return 0;
}


int
menu_runner(Menu *menu)
{
	SDL_Surface *intro;
	Uint32 now, framecount = 0, fps_lastframe = 0;
	Uint8 running = 0;
	Sint32 elapsed;
	SDL_Event event;

	/* Load the initial image and fade into it. */
	intro = loadimage("gfx/gamemenu.png");
	surface_fadein(intro, 8);

	while (running == 0) {
		while (SDL_PollEvent(&event)) {
			running = handle_menu_events(&event, menu);
		}

		now = SDL_GetTicks();

		/* Every 1.0 / MAXFPS seconds, refresh the screen. */
		if (fps_lastframe < (now - (1000/MAXFPS))) {
			framecount++;
			fps_lastframe = now;
			SDL_BlitSurface(intro, NULL, screen, NULL);

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

	menu = menu_new();
	menu->x = 220;
	menu->y = 285;

	menu_load_main(menu);

	status = menu_runner(menu);

	menu_kill(menu);

	return status;
}

