#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "SDL.h"

#include "rezerwar.h"

HiScore **hs = NULL;

/**
 * Called from most function to load the hiscore table in case it isn't already
 * in memory.
 */
void
hiscore_load()
{
	int i;
	size_t len;
	FILE *fp;
	char *buffer;
	char *c;

	if (hs == NULL) {
		hs = malloc(sizeof(HiScore*) * 10);

		for (i = 0; i < 10; i++) {
			hs[i] = malloc(sizeof(HiScore));
			strlcpy(hs[i]->name, "Anonymous", 16);
			hs[i]->score = 0;
			hs[i]->date = 0;
		}

		fp = fopen("hiscore.dat", "r");
		if (fp == NULL)
			return;

		buffer = malloc(512);
		len = fread(buffer, 1, 512, fp);
		buffer[len] = 0;
		fclose(fp);
		printf("size:%d\n", len);

		i = 0;
		while ((c = strsep(&buffer, "\n")) != NULL && i < 10) {
			sscanf(c, "%d:%d:%s", &(hs[i]->score),
					(int *)&(hs[i]->date), hs[i]->name);
			i++;
		}
		free(buffer);
	}
}

/**
 * Dump all the text on the board for the high score.
 */
void
hiscore_dump(Board *board)
{
	int i;
	Text *text;
	char buf[16];

	hiscore_load();

	for (i = 0; i < 10; i++) {
		snprintf(buf, 16, "%d", hs[i]->score);
		text = board_add_text(board, buf, 200, 120 + 24 * i);
		text_set_color1(text, 80, 190, 100);
		text_set_color2(text, 30, 130, 40);
		text = board_add_text(board, hs[i]->name, 350, 120 + 24 * i);
		text_set_color1(text, 80, 100, 190);
		text_set_color2(text, 30, 40, 130);
	}
}

/**
 * Commit to file the current hiscore table.
 */
void
hiscore_save()
{
	int i;
	FILE *fp;

	fp = fopen("hiscore.dat", "w");
	if (fp == NULL) {
		return;
	}

	for (i = 0; i < 10; i++) {
		fprintf(fp, "%d:%u:%s\n", hs[i]->score, 
				(unsigned int)hs[i]->date, hs[i]->name);
	}

	fclose(fp);
}

/**
 * Add a new high score!
 */
void
hiscore_add(char *name, int score)
{
	int i, x;

	hiscore_load();

	/* At the end of this loop, i is the index of the item to replace */
	for (i = 0; i < 10; i++)
		if (score > hs[i]->score)
			break;
	x = i;
	printf("index: %d\n", x);
	
	/* Drop the last item and shift all of them. */
	r_free(hs[9]);
	for (i = 8; i > x; i--)
		hs[i] = hs[i - 1];

	hs[x] = malloc(sizeof(HiScore));
	strlcpy(hs[x]->name, name, 16);
	hs[x]->score = score;
	hs[x]->date = time(NULL);

	hiscore_save();
}

/**
 * Return whether the score is a hiscore or not, it assumes 'hs' is always
 * sorted from the highest to the lowest score.
 */
bool
hiscore_check(int score)
{
	int i;

	hiscore_load();

	for (i = 0; i < 10; i++) {
		if (score > hs[i]->score) {
			return true;
		}
	}

	return false;
}

/**
 * Just sanity function... called from main.
 */
void
hiscore_free()
{
	r_free(hs);
}

