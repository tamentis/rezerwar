/* $Id$
 *
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


#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "rezerwar.h"
#include "config.h"




/**
 * Resolve a relative data path.
 */
char *
dpath(const char *org)
{
	char *output;
	size_t len;

	len = strlen(DATAPATH) + strlen(org) + 2;
	output = r_malloc(len);
	snprintf(output, len, "%s/%s", DATAPATH, org);

	return output;
}


char *
cpath(const char *org)
{
	char *output;
	size_t len;
	struct stat st;

#ifdef CFGPATH
	/* Create the cfg folder if needed */
	char cfgdir[MAXPATHLEN];
	snprintf(cfgdir, MAXPATHLEN, "%s/" CFGPATH, getenv("HOME"));
	if (stat(cfgdir, &st) < 0) {
		if (mkdir(cfgdir, 0755) != 0)
			fatal("Unable to create " CFGPATH);
	}
	len = strlen(cfgdir) + strlen(org) + 2;
	output = r_malloc(len);
	snprintf(output, len, "%s/%s", cfgdir, org);
#else
	len = strlen(org) + 1;
	output = r_malloc(len);
	snprintf(output, len, "%s", org);
#endif

	return output;
}


