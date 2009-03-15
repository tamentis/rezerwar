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
	l->y = 0;
	l->h = FONT_HEIGHT;
	l->w = 13;

	switch(c) {
		case 'a': case 'A': l->x = 160; break;
		case 'b': case 'B': l->x = 172; break;
		case 'c': case 'C': l->x = 184; break;
		case 'd': case 'D': l->x = 197; break;
		case 'e': case 'E': l->x = 210; break;
		case 'f': case 'F': l->x = 223; l->w = 14; break;
		case 'g': case 'G': l->x = 237; break;
		case 'h': case 'H': l->x = 250; break;
		case 'i': case 'I': l->x = 263; l->w = 7; break;
		case 'j': case 'J': l->x = 270; break;
		case 'k': case 'K': l->x = 283; l->w = 14; break;
		case 'l': case 'L': l->x = 297; l->w = 14; break;
		case 'm': case 'M': l->x = 311; l->w = 15; break;
		case 'n': case 'N': l->x = 326; l->w = 15; break;
		case 'o': case 'O': l->x = 340; break;
		case 'p': case 'P': l->x = 353; break;
		case 'q': case 'Q': l->x = 366; l->w = 14; break;
		case 'r': case 'R': l->x = 379; l->w = 15; break;
		case 's': case 'S': l->x = 394; break;
		case 't': case 'T': l->x = 407; l->w = 17; break;
		case 'u': case 'U': l->x = 424; break;
		case 'v': case 'V': l->x = 436; l->w = 14; break;
		case 'w': case 'W': l->x = 450; l->w = 16; break;
		case 'x': case 'X': l->x = 466; l->w = 16; break;
		case 'y': case 'Y': l->x = 482; break;
		case 'z': case 'Z': l->x = 495; break;
		case '0': l->y = FONT_HEIGHT; l->x = 160; l->w = 14; break;
		case '1': l->y = FONT_HEIGHT; l->x = 174; l->w = 7; break;
		case '2': l->y = FONT_HEIGHT; l->x = 181; l->w = 15; break;
		case '3': l->y = FONT_HEIGHT; l->x = 196; l->w = 15; break;
		case '4': l->y = FONT_HEIGHT; l->x = 211; l->w = 15; break;
		case '5': l->y = FONT_HEIGHT; l->x = 226; l->w = 14; break;
		case '6': l->y = FONT_HEIGHT; l->x = 240; l->w = 15; break;
		case '7': l->y = FONT_HEIGHT; l->x = 256; l->w = 14; break;
		case '8': l->y = FONT_HEIGHT; l->x = 269; l->w = 15; break;
		case '9': l->y = FONT_HEIGHT; l->x = 284; l->w = 15; break;
		case '-': l->y = FONT_HEIGHT; l->x = 299; l->w = 11; break;
		case '.': l->y = FONT_HEIGHT; l->x = 310; l->w =  5; break;
		case '/': l->y = FONT_HEIGHT; l->x = 315; l->w = 11; break;
		case '?': l->y = FONT_HEIGHT; l->x = 326; l->w = 13; break;
		case '!': l->y = FONT_HEIGHT; l->x = 339; l->w =  5; break;
		case '\'': l->y = FONT_HEIGHT; l->x = 344; l->w = 6; break;
		case ':': l->y = FONT_HEIGHT; l->x = 350; l->w = 5; break;
		default:  l->y = FONT_HEIGHT; l->x = 356; l->w = 8; break;
	}

	// Offset
	l->y += 128;
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
	int force = 3;

	*rx = (rand() % 50 > 45 ? (rand() % (force + 1)) : 0);
	*ry = (rand() % 50 > 40 ? (rand() % (force + 1)) : 0);
}


/**
 * This effect updates the alpha level of a surface and trash it when it reach
 * the bottom.
 */
void
text_effect_fadeout(Text *text, SDL_Surface *s)
{
	int alpha;

	/* -255 is the uninitialized state. */
	if (text->fx_fade_data == -255)
		text->fx_fade_data = 400;	

	text->fx_fade_data -= 8;

	/* Manage a min/max */
	if (text->fx_fade_data > 255) {
		alpha = 255;
	} else if (text->fx_fade_data < 0) {
		alpha = 0;
		text->trashed = true;
	} else {
		alpha = text->fx_fade_data;
	}

	SDL_SetAlpha(s, SDL_SRCALPHA|SDL_RLEACCEL, alpha);
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
		/* Real Newline */
		if (*c == '\n') {
			cursor = 0;
			ry += FONT_HEIGHT + text->line_spacing;
			c++;
			continue;
		}
		/* Forced Newline */
		if (*c == '\\' && *(c+1) == 'n')  {
			cursor = 0;
			ry += FONT_HEIGHT + text->line_spacing;
			c += 2;
			continue;
		}

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
 * Easy function to set the color1 and color2.
 */
void
text_set_color1(Text *text, byte r, byte g, byte b)
{
	text->colorized = true;
	text->color1_r = r;
	text->color1_g = g;
	text->color1_b = b;
}
void
text_set_color2(Text *text, byte r, byte g, byte b)
{
	text->colorized = true;
	text->color2_r = r;
	text->color2_g = g;
	text->color2_b = b;
}


/**
 * Even easier function to set both colors with Web-friendly Hex values.
 */
void
text_set_colors(Text *text, uint32_t col1, uint32_t col2)
{
	text->colorized = true;
	text->color1_r = (col1 & 0x00FF0000) >> 16;
	text->color1_g = (col1 & 0x0000FF00) >> 8;
	text->color1_b =  col1 & 0x000000FF;

	text->color2_r = (col2 & 0x00FF0000) >> 16;
	text->color2_g = (col2 & 0x0000FF00) >> 8;
	text->color2_b =  col2 & 0x000000FF;
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
		if (*c == '\n' || (*c == '\\' && *(c+1) == 'n'))
			text->height += FONT_HEIGHT;
		c++;
	}
}


Text *
text_new(char *value)
{
	Text *text;

	text = r_malloc(sizeof(Text));

	text->x = 0;
	text->y = 0;
	text->width = 0;
	text->height = FONT_HEIGHT;
	text->effect = 0;
	text->fx_fade_data = -255;
	text->value = NULL;
	text->length = -1;
	text->line_spacing = 2;

	text->trashed = false;
	text->colorized = false;

	text->color1_r = 0xFF;
	text->color1_g = 0xFF;
	text->color1_b = 0xFF;

	text->centered = false;
	
	text_set_value(text, value);

	return text;
}

/**
 * Set the value (text) for this Text entity. If the text is currently empty
 * and we are trying to set another empty value, just return.
 */
void
text_set_value(Text *text, char *value)
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
text_add_char(Text *text, char ch)
{
	char *new_prompt;
	int nlen = text->length + 1;

	new_prompt = r_malloc(nlen + 1);
	strlcpy(new_prompt, (char*)text->value, nlen + 1);
	new_prompt[nlen] = '\0';
	new_prompt[nlen - 1] = ch;
	r_free(text->value);
	text->value = (byte*)new_prompt;
	text->length = nlen;

	text_calculate_size(text);
}

void
text_del_last_char(Text *text)
{
	if (text->length) {
		text->length--;
		text->value[text->length] = '\0';
		text_calculate_size(text);
	}

}

void
text_kill(Text *text)
{
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

	if (text->effect & EFFECT_FADEOUT) {
		text_effect_fadeout(text, s);
	}

	return s;
}


/**
 * Return an SDL Rectangle of the position and size of the text block.
 */
void
text_get_rectangle(Text *text, SDL_Rect *r)
{
	r->w = text->width;
	r->h = FONT_HEIGHT;
	r->y = text->y;
	if (text->centered == true)
		r->x = (screen->w - text->width) / 2;
	else
		r->x = text->x;
}

