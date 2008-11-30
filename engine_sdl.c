#include <stdio.h>

#include "SDL.h"
#include "SDL_image.h"

#include "rezerwar.h"


extern SDL_Surface *screen;


void
r_setpixel(Uint16 x, Uint16 y, Uint8 r, Uint8 g, Uint8 b)
{
	Uint32 *v32;
	Uint32 c32;

	switch (screen->format->BitsPerPixel) {
		case 8:
			printf("8bpp\n");
			break;
		case 16:
			printf("16bpp\n");
			break;
		case 24:
			printf("24bpp\n");
			break;
		case 32:
			c32 = SDL_MapRGBA(screen->format, r, g, b, 100);
//			c32 = SDL_MapRGB(screen->format, r, g, b);
//			printf("32bpp @ %dx%d=%d -> %d\n", x, y, y * screen->w * 4 + x, c32);
//			v32 = (Uint32*)screen->pixels + y * screen->w + x;
			v32 = screen->pixels + (y * screen->w + x) * 4;
			*v32 = c32;
			break;
		default:
			printf("oops... unknown pixel format.\n");
			exit(-1);
	}
}


void
r_setline(Uint16 x, Uint16 y, Uint16 w, Uint8 r, Uint8 g, Uint8 b)
{
	Uint16 i;

	/* Don't bother for single drops. */
	if (w == 1) {
		r_setpixel(x, y, r, g, b);
		return;
	}

	x = x - w / 2;

	for (i = 0; i < w; i++) {
		r_setpixel(x + i, y, r, g, b);
	}
}


void
white_screen()
{
	memset(screen->pixels, 255, screen->w * screen->h *
			screen->format->BytesPerPixel);
	SDL_Flip(screen);
}


void
black_screen(Uint8 s)
{
	memset(screen->pixels, 0, screen->w * screen->h *
			screen->format->BytesPerPixel);
	SDL_Flip(screen);
	SDL_Delay(s * 1000);
}


/**
 * Fade a surface in the screen. If any event occur, return 1 if the fade was
 * aborted by a keystroke.
 */
int
surface_fadein(SDL_Surface *surf, int speed)
{
	Uint8 i;
	int r = 0;
	SDL_Event event;
	int max = 255 / speed;

	for (i = 0; i < max; i++) {
		if (SDL_PollEvent(&event) && event.type == SDL_KEYDOWN) {
			r = 1;
			i = max;
		}

		memset(screen->pixels, 0, screen->w * screen->h *
				screen->format->BytesPerPixel);
		SDL_SetAlpha(surf, SDL_RLEACCEL | SDL_SRCALPHA, i * speed);
		SDL_BlitSurface(surf, NULL, screen, NULL);
		SDL_Flip(screen);
		SDL_Delay(10);
	}

	return r;
}


/**
 * Fade a surface out of the screen (to black). If a keystroke is recorded,
 * return 1.
 */
int
surface_fadeout(SDL_Surface *surf)
{
	Uint8 i;
	SDL_Event event;

	for (i = 0; i < 64; i++) {
		if (SDL_PollEvent(&event) && event.type == SDL_KEYDOWN)
			return 1;

		memset(screen->pixels, 0, screen->w * screen->h *
				screen->format->BytesPerPixel);
		SDL_SetAlpha(surf, SDL_RLEACCEL | SDL_SRCALPHA, 255 - i * 4);
		SDL_BlitSurface(surf, NULL, screen, NULL);
		SDL_Flip(screen);
		SDL_Delay(10);
	}

	return 0;
}


SDL_Surface *
loadimage(char *filename)
{
	SDL_Surface *img;

	img = IMG_Load(filename);

	if (img == NULL) {
		fprintf(stderr, "Unable to load image \"%s\".\n", filename);
		exit(-1);
	}

	return img;
}

