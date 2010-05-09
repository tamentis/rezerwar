/*
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

#include "SDL.h"

#include "rezerwar.h"

/**
 * Level constructor
 */
Level *
lvl_new()
{
	Level *level;

	level = malloc(sizeof(Level));
	level->name = NULL;
	level->description = NULL;
	level->next = NULL;
	level->queue = NULL;
	level->queue_len = 0;
	level->allow_bomb = true;
	level->allow_medic = true;
	level->rising_speed = -1;
	level->time_limit = -1;
	level->max_moles = 0;
	level->max_cubes = 0;
	level->dead_pipes = 0;
	level->cmap = malloc(sizeof(byte) * BOARD_WIDTH * BOARD_HEIGHT);

	return level;
}

/**
 * Level destructor
 */
void
lvl_kill(Level *level)
{
	r_free(level->name);
	free(level->description);
	r_free(level);
}


/*************************
* LEVEL PSEUDO VARIABLES *
*************************/

/* $ObjectiveType */
void
lvl_var_objtype(Level *level, char *value)
{
	if (strcmp(value, "CLEARALL") == 0) {
		level->objective_type = OBJTYPE_CLEARALL;
	} else if (strcmp(value, "LINK") == 0) {
		level->objective_type = OBJTYPE_LINK;
	} else {
		fatal("Unknown value for ObjectiveType");
	}
}

/* $NextLevel */
void
lvl_var_nextlevel(Level *level, char *value)
{
	size_t len = strlen(value) + 1;

	level->next = malloc(len);
	strlcpy(level->next, value, len);
}

/* $AllowDynamite */
void
lvl_var_allowdynamite(Level *level, const char *value)
{
	level->allow_bomb = lvl_bool(value);
}

/* $AllowMedic */
void
lvl_var_allowmedic(Level *level, const char *value)
{
	level->allow_medic = lvl_bool(value);
}

/* $DeadPipes */
void
lvl_var_deadpipes(Level *level, char *value)
{
	level->dead_pipes = atoi(value);
}

/* $MaxCubesAllowed */
void
lvl_var_maxcubesallowed(Level *level, char *value)
{
	level->max_cubes = atoi(value);
}

/* $MaxMoles */
void
lvl_var_maxmoles(Level *level, char *value)
{
	level->max_moles = atoi(value);
}

/* $TimeLimit */
void
lvl_var_timelimit(Level *level, char *value)
{
	level->time_limit = atoi(value);
}

/* $RisingSpeed */
void
lvl_var_risingspeed(Level *level, char *value)
{
	level->rising_speed = atoi(value);
}


/**
 * Copy the next line of *buf in *lbuf, assuming it has enough space and
 * return the number of char we moved including the new line.
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
 * Get a variable line buffer, split it and assign the proper variable.
 */
void
lvl_splitvar(Level *level, byte *lbuf, size_t len)
{
	char *c, *l = (char *)lbuf;

	/* Find the first colon (non optional) and terminal the string to
	 * find the variable name, increment c to skip the colon/NUL */
	c = strchr(l, ':');
	if (c == NULL) fatal("Syntax error: variable without ':'.");
	*c = '\0';

	/* Skip the NUL/:, the spaces... */
	c++; while (*c == ' ') c++;

	if (strcmp("ObjectiveType", l) == 0)
		lvl_var_objtype(level, c);
	else if (strcmp("NextLevel", l) == 0)
		lvl_var_nextlevel(level, c);
	else if (strcmp("AllowDynamite", l) == 0)
		lvl_var_allowdynamite(level, c);
	else if (strcmp("AllowMedic", l) == 0)
		lvl_var_allowmedic(level, c);
	else if (strcmp("MaxCubesAllowed", l) == 0)
		lvl_var_maxcubesallowed(level, c);
	else if (strcmp("MaxMoles", l) == 0)
		lvl_var_maxmoles(level, c);
	else if (strcmp("DeadPipes", l) == 0)
		lvl_var_deadpipes(level, c);
	else if (strcmp("TimeLimit", l) == 0)
		lvl_var_timelimit(level, c);
	else if (strcmp("RisingSpeed", l) == 0)
		lvl_var_risingspeed(level, c);
	else
		fatal("Syntax error: unknown variable: \"%s\".", l);
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
	char *path;
	byte buffer[LVL_MAX_SIZE];
	byte lbuf[81];
	byte *cursor = buffer;
	byte *c;
	size_t len, offset, lineno = 1;
	int phase = 0, i, j;

	snprintf(filename, 64, "levels/%s.lvl", name);
	path = dpath(filename);

	fp = fopen(path, "r");
	if (fp == NULL)
		fatal("Error opening \"%s\"", name);
	r_free(path);

	len = fread(buffer, 1, LVL_MAX_SIZE, fp);
	if (len >= LVL_MAX_SIZE)
		fatal("Level file too big (LVL_MAX_SIZE=%d)", LVL_MAX_SIZE);

	fclose(fp);

	level = lvl_new();

	for (i = 0; cursor < buffer + len;) {
		/* offset becomes the total length of the string including the
		 * terminating NUL. */
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
			level->name = r_malloc(offset);
			strlcpy(level->name, (char *)lbuf, offset);
			phase++;
			cursor++; // skip next blank line
			continue;
		}

		/* Save the description (phase 1) */
		if (phase == 1) {
			if (i > 0) level->description[i - 1] = '\n';
			level->description = realloc(level->description, 
					i + offset);
			strlcpy(level->description + i, (char*)lbuf, offset);
			i += offset;
		}

		/* Save the map */
		if (phase == 2) {
			if (offset - 1 > BOARD_WIDTH)
				fatal("Syntax error: map line has to be %d wide.", BOARD_WIDTH);
			memcpy(level->cmap + i, lbuf, offset - 1);
			i += offset - 1;
			if (i > BOARD_WIDTH * BOARD_HEIGHT)
				fatal("map seems bigger than board.");
		}

		/* Save the queue */
		if (phase == 3) {
			if (offset < 4)
				fatal("Syntax error on line %d: cube definition erroneous.", lineno);
			level->queue = realloc(level->queue, 
					sizeof(QueuedCube*) * (i + 1));
			level->queue[i] = malloc(sizeof(QueuedCube));
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

		/* Variables */
		if (lbuf[0] == '$') {
			lvl_splitvar(level, lbuf + 1, offset - 2);
		}
	}

	return level;
}

/**
 * Debugging tool for Level
 */
void
lvl_dump(Level *level)
{
	int i, j;

	printf("NAME: %s\n", level->name);
	printf("DESCRIPTION:\n%s\n", level->description);
	printf("MAP:%s\n", level->cmap);
	printf("QUEUE\n");

	printf("ALLOW_BOMB: %d\n", level->allow_bomb);
	printf("ALLOW_MEDIC: %d\n", level->allow_medic);
	printf("MOLES: %d\n", level->max_moles);
	printf("DEADPIPES: %d\n", level->dead_pipes);

	for (i = 0; i < level->queue_len; i++) {
		printf(" - type=%d, pos=%d, cubes(%zu)=",
				level->queue[i]->type,
				level->queue[i]->pos,
				level->queue[i]->cmap_len);
		for (j = 0; j < level->queue[i]->cmap_len; j++) {
			printf("%02hhx ", level->queue[i]->cmap[j]);
		}
		printf("\n");
	}
}

bool
lvl_bool(const char *value)
{
	if (strcmp(value, "TRUE") == 0) {
		return true;
	} else if (strcmp(value, "FALSE") == 0) {
		return false;
	}

	fatal("Unknown value for BOOLEAN: %s", value);
	return false;
}
