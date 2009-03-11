#include <stdio.h>
#include <stdlib.h>

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
			fatal("Syntax error: line too long! (MAX=80)");
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
	byte *c;
	size_t len, offset, lineno = 1;
	int phase = 0, i, j;

	snprintf(filename, 64, "levels/%s.lvl", name);

	fp = fopen(filename, "r");
	if (fp == NULL)
		fatal("Error opening \"%s\"", name);

	len = fread(buffer, 1, LVL_MAX_SIZE, fp);
	if (len >= LVL_MAX_SIZE)
		fatal("Level file too big (LVL_MAX_SIZE=%d)", LVL_MAX_SIZE);

	fclose(fp);

	level = malloc(sizeof(Level));
	level->name = NULL;
	level->description = NULL;
	level->queue = NULL;
	level->queue_len = 0;
	level->cmap = malloc(sizeof(byte) * BOARD_WIDTH * BOARD_HEIGHT);

	for (i = 0; cursor < buffer + len;) {
		offset = lvl_getline(lbuf, cursor);
		if (offset == -1)
			break;
		lineno++;
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
			i = 0;
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
					i + offset);
			memcpy(level->description + i, lbuf, offset);
			level->description[i + offset - 1] = '\n';
			i += offset;
		}

		/* Save the map */
		if (phase == 2) {
			if (offset - 1 > BOARD_WIDTH)
				fatal("Syntax error: map line has to be %d wide.", BOARD_WIDTH);
			memcpy(level->cmap + i, lbuf, offset);
			i += offset - 1;
			if (i > BOARD_WIDTH * BOARD_HEIGHT)
				fatal("map seems bigger than board.");
		}

		/* Save the queue */
		if (phase == 3) {
			if (offset < 4)
				fatal("Syntax error on line %d: cube definition erroneous.", lineno);
			level->queue = realloc(level->queue, 
					sizeof(QueuedBlock*) * (i + 1));
			level->queue[i] = malloc(sizeof(QueuedBlock));
			level->queue[i]->type = lbuf[0] - 48;
			level->queue[i]->pos = lbuf[1] - 48;
			level->queue[i]->cmap_len = offset - 3;
			level->queue[i]->cmap = malloc(offset - 3);
			c = lbuf + 2;
			j = 0;
			while (*c != '\n' && j < offset - 3) {
				level->queue[i]->cmap[j] = c[j];
				j++;
			}
			i++;
			level->queue_len++;
		}
	}

	return level;
}

void
lvl_dump(Level *level)
{
	int i, j;

	printf("NAME: %s\n", level->name);
	printf("DESCRIPTION:\n%s\n", level->description);
	printf("MAP:%s\n", level->cmap);
	printf("QUEUE\n");

	for (i = 0; i < level->queue_len; i++) {
		printf(" - type=%d, pos=%d, cubes(%d)=",
				level->queue[i]->type,
				level->queue[i]->pos,
				level->queue[i]->cmap_len);
		for (j = 0; j < level->queue[i]->cmap_len; j++) {
			printf("%02hhx ", level->queue[i]->cmap[j]);
		}
		printf("\n");
	}
}

void
lvl_kill(Level *level)
{
	free(level->name);
	free(level->description);
}

