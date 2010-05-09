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


#include <stdlib.h>
#include <stdio.h>
#include "SDL.h"
#include "rezerwar.h"


/* Keep records. */
uint32_t total = 0;
uint32_t current = 0;
uint32_t max = 0;
uint32_t current_c = 0;


/* What will hold the list of pointers, and the cursor to the last one. */
size_t *slist = NULL;
void **list = NULL;
unsigned list_size = 0;


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


/**
 * Copy a NUL delimited string and return the fresh copy.
 */
char *
r_strcp(char *src)
{
	size_t len = strlen(src) + 1;
	char *dst = r_malloc(len);
	strlcpy(dst, src, len);
	return dst;
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
