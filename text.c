#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"

#include "rezerwar.h"


extern SDL_Surface *screen;
extern SDL_Surface *sprites;
extern Uint32 key;

void
font_get_glyph_rect(char c, SDL_Rect *l)
{
	l->y = 24;
	l->h = 19;
	l->w = 13;

	switch(c) {
		case 'a': case 'A': l->x = 96; break;
		case 'b': case 'B': l->x = 108; break;
		case 'c': case 'C': l->x = 120; break;
		case 'd': case 'D': l->x = 133; break;
		case 'e': case 'E': l->x = 146; break;
		case 'f': case 'F': l->x = 159; l->w = 14; break;
		case 'g': case 'G': l->x = 173; break;
		case 'h': case 'H': l->x = 186; break;
		case 'i': case 'I': l->x = 199; l->w = 7; break;
		case 'j': case 'J': l->x = 206; break;
		case 'k': case 'K': l->x = 219; l->w = 14; break;
		case 'l': case 'L': l->x = 233; l->w = 14; break;
		case 'm': case 'M': l->x = 247; l->w = 15; break;
		case 'n': case 'N': l->x = 262; l->w = 15; break;
		case 'o': case 'O': l->x = 276; break;
		case 'p': case 'P': l->x = 289; break;
		case 'q': case 'Q': l->x = 302; l->w = 14; break;
		case 'r': case 'R': l->x = 315; l->w = 15; break;
		case 's': case 'S': l->x = 330; break;
		case 't': case 'T': l->x = 343; l->w = 17; break;
		case 'u': case 'U': l->x = 360; break;
		case 'v': case 'V': l->x = 372; l->w = 14; break;
		case 'w': case 'W': l->x = 386; l->w = 16; break;
		case 'x': case 'X': l->x = 402; l->w = 16; break;
		case 'y': case 'Y': l->x = 418; break;
		case 'z': case 'Z': l->x = 431; break;
		case '0': l->y = 43; l->x = 96; l->w = 14; break;
		case '1': l->y = 43; l->x = 110; l->w = 7; break;
		case '2': l->y = 43; l->x = 117; l->w = 15; break;
		case '3': l->y = 43; l->x = 132; l->w = 15; break;
		case '4': l->y = 43; l->x = 147; l->w = 15; break;
		case '5': l->y = 43; l->x = 162; l->w = 14; break;
		case '6': l->y = 43; l->x = 176; l->w = 15; break;
		case '7': l->y = 43; l->x = 192; l->w = 14; break;
		case '8': l->y = 43; l->x = 205; l->w = 15; break;
		case '9': l->y = 43; l->x = 220; l->w = 15; break;
		case '-': l->y = 43; l->x = 235; l->w = 11; break;
		case '.': l->y = 43; l->x = 246; l->w =  5; break;
		case '/': l->y = 43; l->x = 251; l->w = 11; break;
		case '?': l->y = 43; l->x = 262; l->w = 13; break;
		case '!': l->y = 43; l->x = 275; l->w =  5; break;
		case '\'': l->y = 43; l->x = 280; l->w =  6; break;
		case ':': l->y = 43; l->x = 286; l->w =  5; break;
		default: l->x = 444; l->w = 8; break;
	}
}


/**
 * This function prints a single letter on screen and return x+width of the char
 */
int
text_render_glyph(SDL_Surface *s, char c, int x, int y)
{
	SDL_Rect lr, dr;

	dr.x = x;
	dr.y = y;

	font_get_glyph_rect(c, &lr);
	dr.w = lr.w;
	dr.h = lr.h;
	SDL_BlitSurface(sprites, &lr, s, &dr);

	return lr.w;
}


/**
 * This effect will return a different offset for x and y for each letters.
 */
void
text_effect_shake(Text *text, int *rx, int *ry)
{
	int force = 2;
	int speed = 4;

	if (text->fx_data == NULL) {
		text->fx_data = r_malloc(sizeof(int));
		text->fx_data[0] = speed;
	}

	if (text->fx_data[0] < 1) {
		text->fx_data[0] = speed;
		*rx = (rand() % 50 > 25 ? (rand() % (force + 1)) : 0);
		*ry = (rand() % 50 > 25 ? (rand() % (force + 1)) : 0);
	} else {
		*rx = 0;
		*ry = 0;
	}

	text->fx_data[0]--;
}


/**
 * Try to colorize. TODO, make this not 32-bit only.
 */
void
text_effect_colorize(Text *text, SDL_Surface *s)
{
	int i, max = s->w * s->h;
	byte r, g, b;
	Uint32 *c;

	for (i = 0; i < max; i++) {
		c = s->pixels + i * s->format->BytesPerPixel;
		SDL_GetRGB(*c, s->format, &r, &g, &b);

		if ((r & g & b) == 255) 
			*c = SDL_MapRGB(s->format, text->color1_r, 
					text->color1_g, text->color1_b);

		if ((r | g | b) == 0)
			*c = SDL_MapRGB(s->format, text->color2_r, 
					text->color2_g, text->color2_b);
	}
}


/**
 * This function will simply print the 'text' at the given coordinate.
 */
void
text_render(Text *text, SDL_Surface *s)
{
	unsigned char *c = text->value;
	int cursor = 0;
	int rx = 0, ry = 0;

	while (*c != '\0') {
		if (text->effect & EFFECT_SHAKE) {
			text_effect_shake(text, &rx, &ry);
		}
		cursor += text_render_glyph(s, *c, cursor + rx, 0 + ry);
		c++;
	}

	if (text->colorized == true)
		text_effect_colorize(text, s);
}


/**
 * Easy function to set the color1.
 */
void
text_set_color1(Text *text, byte r, byte g, byte b)
{
	text->colorized = true;
	text->color1_r = r;
	text->color1_g = g;
	text->color1_b = b;
}


/**
 * Easy function to set the color2.
 */
void
text_set_color2(Text *text, byte r, byte g, byte b)
{
	text->colorized = true;
	text->color2_r = r;
	text->color2_g = g;
	text->color2_b = b;
}


/**
 * Refresh the dimensions of the block from the glyphs.
 */
void
text_calculate_size(Text *text)
{
	SDL_Rect r;
	unsigned char *c = text->value;

	text->width = 0;
	while (*c != '\0') {
		font_get_glyph_rect(*c, &r);
		text->width += r.w;
		c++;
	}
}


Text *
text_new(unsigned char *value)
{
	Text *text;

	text = r_malloc(sizeof(Text));

	text->x = 0;
	text->y = 0;
	text->width = 0;
	text->height = 19;
	text->effect = 0;
	text->fx_data = NULL;
	text->value = NULL;
	text->length = -1;

	text->colorized = false;

	text->color1_r = 0xFF;
	text->color1_g = 0xFF;
	text->color1_b = 0xFF;
	
	text_set_value(text, value);

	return text;
}


/**
 * Set the value (text) for this Text entity. If the text is currently empty
 * and we are trying to set another empty value, just return.
 */
void
text_set_value(Text *text, unsigned char *value)
{
	if (value[0] == '\0' && text->length == 0)
		return;

	text->length = strlen((char *)value);
	r_free(text->value);
	text->value = r_malloc(text->length + 1);
	strlcpy((char *)text->value, (char *)value, text->length + 1);

	text_calculate_size(text);
}


void
text_kill(Text *text)
{
	r_free(text->fx_data);
	r_free(text->value);
	r_free(text);
}


/**
 * Return an SDL Surface of the rendered text, at this point in time.
 */
SDL_Surface *
text_get_surface(Text *text)
{
	SDL_Surface *s;

	s = SDL_CreateRGBSurface(0, text->width, text->height, 
			screen->format->BitsPerPixel, 0, 0, 0, 0);
	SDL_FillRect(s, NULL, key);
	SDL_SetColorKey(s, SDL_SRCCOLORKEY|SDL_RLEACCEL, key);
	text_render(text, s);

	return s;
}


/**
 * Return an SDL Rectangle of the position and size of the text block.
 */
void
text_get_rectangle(Text *text, SDL_Rect *r)
{
	r->w = 200;
	r->h = 19;
	r->x = text->x;
	r->y = text->y;
}

