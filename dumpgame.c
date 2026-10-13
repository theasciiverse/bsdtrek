/*-
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @(#)dumpgame.c	8.1 (Berkeley) 5/31/93
 * $FreeBSD: src/games/trek/dumpgame.c,v 1.6 1999/11/30 03:49:46 billf Exp $
 * $DragonFly: src/games/trek/dumpgame.c,v 1.3 2006/09/07 21:19:44 pavalos Exp $
 */

#include <fcntl.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "trek.h"

#ifndef O_BINARY
#define O_BINARY	0
#endif
#ifdef _WIN32
#include <io.h>
#define MKDIR(p)	mkdir(p)
#else
#define MKDIR(p)	mkdir(p, 0755)
#endif

/***  THIS CONSTANT MUST CHANGE AS THE DATA SPACES CHANGE ***/
#define VERSION		2

struct dump {
	char	*area;
	int	count;
};

static bool readdump(int);
static const char *dumpfile(bool);

struct dump Dump_template[] = {
	{ (char *)&Ship,	sizeof (Ship)	},
	{ (char *)&Now,		sizeof (Now)	},
	{ (char *)&Param,	sizeof (Param)	},
	{ (char *)&Etc,		sizeof (Etc)	},
	{ (char *)&Game,	sizeof (Game)	},
	{ (char *)Sect,		sizeof (Sect)	},
	{ (char *)Quad,		sizeof (Quad)	},
	{ (char *)&Move,	sizeof (Move)	},
	{ (char *)Event,	sizeof (Event)	},
	{ NULL,			0		}
};

/*
**  DUMP GAME
**
**	This routine dumps the game onto the file "trek.dump".  The
**	first two bytes of the file are a version number, which
**	reflects whether this image may be used.  Obviously, it must
**	change as the size, content, or order of the data structures
**	output change.
*/

void
dumpgame(int v __unused)
{
	int		version;
	int		fd;
	struct dump	*d;
	int		i;

	if ((fd = open(dumpfile(true), O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644)) < 0) {
		printf("cannot dump\n");
		return;
	}
	version = VERSION;
	write(fd, &version, sizeof version);

	/* output the main data areas */
	for (d = Dump_template; d->area; d++) {
		write(fd, &d->area, sizeof d->area);
		i = d->count;
		write(fd, d->area, i);
	}

	close(fd);
}


/*
**  RESTORE GAME
**
**	The game is restored from the file "trek.dump".  In order for
**	this to succeed, the file must exist and be readable, must
**	have the correct version number, and must have all the appro-
**	priate data areas.
**
**	Return value is zero for success, one for failure.
*/

bool
restartgame(void)
{
	int	fd;
	int		version;

	if ((fd = open(dumpfile(false), O_RDONLY | O_BINARY)) < 0 ||
	    read(fd, &version, sizeof version) != sizeof version ||
	    version != VERSION ||
	    readdump(fd)) {
		printf("cannot restart\n");
		close(fd);
		return (1);
	}

	close(fd);
	return (0);
}


/*
**  DUMP FILE
**
**	The dump lives in the player's data folder, bsdtrek/trek.dump
**	under $XDG_DATA_HOME, else ~/.local/share, else %APPDATA% on
**	Windows, so that it survives reinstalling the game.  With none
**	of those set it is "trek.dump" in the current directory, as it
**	always was.  With create, the folders are made as needed.
*/

static const char *
dumpfile(bool create)
{
	static char	path[1024];
	const char	*base;
	char		*p;

	if ((base = getenv("XDG_DATA_HOME")) != NULL && *base)
		snprintf(path, sizeof path, "%s/bsdtrek", base);
	else if ((base = getenv("HOME")) != NULL && *base)
		snprintf(path, sizeof path, "%s/.local/share/bsdtrek", base);
	else if ((base = getenv("APPDATA")) != NULL && *base)
		snprintf(path, sizeof path, "%s/bsdtrek", base);
	else
		return ("trek.dump");
	if (create)
		for (p = path + 1; ; p++)
			if (*p == '/' || *p == '\\' || *p == '\0') {
				char c = *p;
				*p = '\0';
				MKDIR(path);
				if ((*p = c) == '\0')
					break;
			}
	strncat(path, "/trek.dump", sizeof path - strlen(path) - 1);
	return (path);
}


/*
**  READ DUMP
**
**	This is the business end of restartgame().  It reads in the
**	areas.
**
**	Returns zero for success, one for failure.
*/

static bool
readdump(int fd1)
{
	int		fd;
	struct dump	*d;
	int		i;
	char		*tag;
	char		*oldevent = NULL;
	ptrdiff_t	n;

	fd = fd1;

	/*
	 * Each area is tagged with its address in the program that wrote
	 * the dump.  Programs now load at a different address every run,
	 * so the tags no longer have to match; the tag of Event is kept
	 * to move the event pointers in Now to this run's Event.
	 */
	for (d = Dump_template; d->area; d++) {
		if (read(fd, &tag, sizeof tag) != (sizeof tag))
			return (1);
		if (d->area == (char *)Event)
			oldevent = tag;
		i = d->count;
		if (read(fd, d->area, i) != i)
			return (1);
	}

	/* make quite certain we are at EOF */
	if (read(fd, &tag, 1))
		return (1);

	for (i = 0; i < NEVENTS; i++) {
		if (Now.eventptr[i] == NULL)
			continue;
		n = ((char *)Now.eventptr[i] - oldevent) / (ptrdiff_t)sizeof Event[0];
		if (n < 0 || n >= MAXEVENTS)
			return (1);
		Now.eventptr[i] = &Event[n];
	}
	Ship.shipname = Ship.ship == QUEENE ? "Fairie Queene" : "Enterprise";
	return (0);
}
