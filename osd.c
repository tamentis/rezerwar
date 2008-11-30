#include <stdio.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


extern SDL_Surface *screen;
extern SDL_Surface *sprites;



void
osd_get_letter_rect(char c, SDL_Rect *l)
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
osd_print_letter(char c, int x, int y)
{
	SDL_Rect lr, dr;

	dr.x = x;
	dr.y = y;

	osd_get_letter_rect(c, &lr);
	dr.w = lr.w;
	dr.h = lr.h;
	SDL_BlitSurface(sprites, &lr, screen, &dr);

	return lr.w;
}



/**
 * This function will simply print the 'text' at the given coordinate.
 */
void
osd_print(char *text, int x, int y)
{
	char *c = text;
	int cursor = 0;

	while (*c != '\0') {
		cursor += osd_print_letter(*c, x + cursor, y);
		c++;
	}
}
