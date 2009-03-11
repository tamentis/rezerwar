#include <stdlib.h>
#include <stdio.h>
#include "SDL.h"


/* Keep records. */
uint32_t total = 0;
uint32_t current = 0;
uint32_t max = 0;
uint32_t current_c = 0;


/* What will hold the list of pointers, and the cursor to the last one. */
size_t *slist = NULL;
void **list = NULL;
size_t list_size = 0;


void *
r_malloc(size_t size)
{
	void *p;

	list_size++;
	current_c++;

	list = realloc(list, list_size * sizeof(void *));
	slist = realloc(slist, list_size * sizeof(size_t));

	p = malloc(size);

	/* Save records of pointer and size. */
	list[list_size - 1] = p;
	slist[list_size - 1] = size;

	if (p == NULL) {
		fprintf(stderr, "Unable to allocate memory.\n");
		exit(0);
	}

	total += size;
	current += size;

	if (current > max) {
		max = current;
	}

	return p;
}

void
r_free(void *p)
{
	size_t i;

	if (p == NULL)
		return;

	for (i = 0; i < list_size; i++) {
		if (list[i] == p) {
			free(p);
			current -= slist[i];
			current_c--;
			list[i] = NULL;
			slist[i] = 0;
			return;
		}
	}
}

void
r_checkmem()
{
	fprintf(stderr, "Maximum amount of memory allocated: %d\n", max);
	fprintf(stderr, "  Total amount of memory allocated: %d\n", total);
	fprintf(stderr, "        Number of calls to rmalloc: %d\n", list_size);
	fprintf(stderr, "      Number of calls left to free: %d\n", current_c);
	fprintf(stderr, "  Amount of memory still allocated: %d\n", current);
}
