/*                        G E T C W D . C
 * BRL-CAD
 *
 * Copyright (c) 2011-2025 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file libbu/getcwd.c
 *
 * Routine(s) for getting the current working directory full pathname.
 *
 */

#include "common.h"

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_DIRECT_H
#  include <direct.h>
#endif
#ifdef HAVE_SYS_PARAM_H /* for MAXPATHLEN */
#  include <sys/param.h>
#endif
#include <string.h>

#include "bu/app.h"
#include "bu/file.h"
#include "bu/malloc.h"
#include "bu/str.h"
#include "bu/log.h"
#include "bu/exit.h"
#include "bu/parallel.h"


/* c89 strict doesn't declare realpath */
#ifndef HAVE_DECL_REALPATH
extern char *realpath(const char *, char *);
#endif


int BU_SEM_DIR;


char *
bu_getcwd(char *buf, size_t size)
{
    char *cwd = NULL;
    char *pwd = NULL;
    char cbuf[MAXPATHLEN] = {0};

    /* NULL buf means allocate */
    if (!buf) {
	size = MAXPATHLEN;
	buf = (char *)bu_calloc(1, size, "alloc bu_getcwd");
    }

    /* FIRST: try getcwd */
#ifdef HAVE_GETCWD
    bu_semaphore_acquire(BU_SEM_SYSCALL);
    cwd = getcwd(cbuf, MAXPATHLEN);
    bu_semaphore_release(BU_SEM_SYSCALL);
    if (cwd
	&& strlen(cwd) > 0
	&& bu_file_exists(cwd, NULL))
    {
#ifdef HAVE_REALPATH
	/* FIXME: shouldn't have gotten here with -std=c99 (HAVE_REALPATH test not working right?) */
	char rbuf[MAXPATHLEN] = {0};
	char *rwd = bu_file_realpath(cbuf, rbuf);
	if (rwd
	    && strlen(rwd) > 0
	    && bu_file_exists(rwd, NULL))
	{
	    bu_strlcpy(buf, rwd, size);
	    return buf;
	}
#endif /* HAVE_REALPATH */
	bu_strlcpy(buf, cwd, size);
	return buf;
    }
#else
    /* quellage */
    cwd = memset(cbuf, 0, MAXPATHLEN);
#endif /* HAVE_GETCWD */


    /* SECOND: try environment */
    pwd = getenv("PWD");
    if (pwd
	&& strlen(pwd) > 0
	&& bu_file_exists(pwd, NULL))
    {
#ifdef HAVE_REALPATH
	char rbuf[MAXPATHLEN] = {0};
	char *rwd = realpath(pwd, rbuf);
	if (rwd
	    && strlen(rwd) > 0
	    && bu_file_exists(rwd, NULL))
	{
	    bu_strlcpy(buf, rwd, size);
	    return buf;
	}
#endif /* HAVE_REALPATH */
	bu_strlcpy(buf, pwd, size);
	return buf;
    }

    /* LAST: punt (but do not return NULL) */
    bu_strlcpy(buf, ".", size);

    return buf;
}


/* set on the first call to bu_getiwd() */
static char iwd[MAXPATHLEN] = {0};

char *
bu_getiwd(char *buf, size_t size)
{
    /* first call initializes */
    bu_semaphore_acquire(BU_SEM_DIR);
    if (iwd[0] == '\0')
	bu_getcwd(iwd, MAXPATHLEN);
    bu_semaphore_release(BU_SEM_DIR);

    /* NULL buf means allocate */
    if (!buf) {
	size = MAXPATHLEN;
	buf = (char *)bu_calloc(1, size, "alloc bu_getcwd");
    }

    bu_strlcpy(buf, iwd, size);

    return buf;
}


/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
