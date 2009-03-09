#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "SDL.h"

#include "rezerwar.h"

/**
 * Copy the next line of *buf in *lbuf, assuming it has enough space and return
 * the number of char we moved including the new line.
 */
size_t
lvl_getline(byte *lbuf, byte *buf)
{
	size_t len = 0;

	if (buf[len] == '\0')
		return -1;
	
	while (buf[len] != '\n' && buf[len] != '\0') {
		lbuf[len] = buf[len];
		len++;
		if (len > 80)
			errx(-1, "Syntax error: line too long! (MAX=80)");
	}
	lbuf[len] = '\0';

	return len + 1;
}


/**
 * Return a freshly loaded level.
 */
Level *
lvl_load(char *name)
{
	Level *level;
	FILE *fp;
	char filename[64];
	byte buffer[LVL_MAX_SIZE];
	byte lbuf[81];
	byte *cursor = buffer;
	size_t len, offset;
	int phase = 0;

	snprintf(filename, 64, "levels/%s.lvl", name);

	fp = fopen(filename, "r");
	if (fp == NULL)
		err(-1, "Error opening \"%s\"", name);

	len = fread(buffer, 1, LVL_MAX_SIZE, fp);
	if (len >= LVL_MAX_SIZE)
		errx(-1, "Level file too big (LVL_MAX_SIZE=%d)", LVL_MAX_SIZE);

	fclose(fp);

	level = malloc(sizeof(Level));
	level->name = NULL;
	level->description = NULL;
	level->queue = NULL;
	level->cmap = malloc(sizeof(byte) * BOARD_WIDTH * BOARD_HEIGHT);

	len = 0;
	for (;;) {
		offset = lvl_getline(lbuf, cursor);
		if (offset == -1)
			break;
		cursor += offset;

		/* Skip comments */
		if (lbuf[0] == '#')
			continue;

		/* Skip the blank lines before the name */
		if (phase == 0 && lbuf[0] == '\0')
			continue;

		/* Delimitors between the phases */
		if (lbuf[0] == '\0') {
			phase++;
			len = 0;
			continue;
		}

		/* Save the name (phase 0) */
		if (level->name == NULL) {
			level->name = malloc(offset + 1);
			strlcpy(level->name, (char *)lbuf, offset + 1);
			phase++;
			cursor++; // skip next blank line
			continue;
		}

		/* Save the description (phase 1) */
		if (phase == 1) {
			level->description = realloc(level->description, 
					len + offset);
			memcpy(level->description + len, lbuf, offset);
			level->description[len + offset - 1] = '\n';
			len += offset;
		}

		/* Save the map */
		if (phase == 2) {
			if (offset - 1 > BOARD_WIDTH)
				errx(-1, "Syntax error: map line has to be %d wide.", BOARD_WIDTH);
			memcpy(level->cmap + len, lbuf, offset);
			len += offset - 1;
			if (len > BOARD_WIDTH * BOARD_HEIGHT)
				errx(-1, "map seems bigger than board.");
		}

		/* Save the queue */
		if (phase == 3) {
			printf("SAVING CUBE ORDER\n");
		}
	}

	return level;
}

void
lvl_dump(Level *level)
{
	printf("NAME: %s\n", level->name);
	printf("DESCRIPTION:\n%s\n", level->description);
	printf("MAP:%s\n", level->cmap);
}

void
lvl_kill(Level *level)
{
	free(level->name);
	free(level->description);
}

