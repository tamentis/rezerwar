#include <stdio.h>
#include <time.h>

#include "SDL.h"

#include "rezerwar.h"

enum toggle_types {
	TOGGLE_DIFFICULTY,
	TOGGLE_SOUND
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
	add_item_to_menu(menu, "replay level", MTYPE_START, 0, 0);
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
	MenuItem *diff_item, *sound_item;

	flush_menu_items(menu);
	diff_item = add_item_to_menu(menu, "difficulty", MTYPE_TOGGLE, 
			TOGGLE_DIFFICULTY, 0);
	sound_item = add_item_to_menu(menu, "sound/music", MTYPE_TOGGLE,
			TOGGLE_SOUND, 0);
	add_item_to_menu(menu, "set controls", MTYPE_NOP, 0, 0);
	add_item_to_menu(menu, "back to main", MTYPE_SUBMENU, 0, 0);

	menu_item_set_difficulty(diff_item);
	menu_item_set_sound(sound_item);
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
		case TOGGLE_SOUND:
			printf("TOGGLE_SOUND!\n");
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
int
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
		text_get_rectangle(item->text, &(item->rect));
		SDL_BlitSurface(s, NULL, screen, &(item->rect));
		SDL_FreeSurface(s);
	}
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

	if (event->type != SDL_KEYDOWN)
		return 0;


	switch ((int)event->key.keysym.sym) {
		case SDLK_j:
		case SDLK_DOWN:
			menu->current++;
			sfx_play_menunav();
			if (menu->current >= menu->length)
				menu->current = 0;
			break;
		case SDLK_UP:
		case SDLK_k:
			menu->current--;
			sfx_play_menunav();
			if (menu->current < 0)
				menu->current = menu->length - 1;
			break;
		case SDLK_RETURN:
			return menu_select(menu);
		case SDLK_ESCAPE:
		case SDLK_q:
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
	menu->bg_image = r_strcp("gfx/gamemenu.bmp");

	menu_load_main(menu);

	sfx_play_music("menu");
	status = menu_runner(menu);

	menu_kill(menu);

	return status;
}


int
gameover_menu(bool success)
{
	Menu *menu;
	Text *title;
	int status;
	bool allow_next_level = true;

	if (conf->next_level == NULL || success == false)
		allow_next_level = false;

	/* Dump a modal and a title before running the menu */
	blit_modal(160);
	if (success == true)
		title = text_new("Congratulations!");
	else
		title = text_new("You failed!");
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
