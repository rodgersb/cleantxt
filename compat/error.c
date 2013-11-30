/*  cleantxt: Cleans up tab, space and end-of-line formatting in text files.
    Copyright (C) 1999-2013 Bryan Rodgers <rodgersb@it.net.au>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at
    your option) any later version.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>. */

/** @file error.c
    Contains a substitute implementation of GNU libc's @c error()
    function for other C libraries that lack it. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#if !defined(HAVE_ERROR) && !defined(_GNU_SOURCE)

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_PROGRAM_INVOCATION_SHORT_NAME
#   include <errno.h>
#else
#   include "progname.h"
#endif

#include "error.h"

void error(int status, int errnum, const char *message, ...)
{
    /* ap: Variable argument pointer */
    va_list ap;

    fputs(program_invocation_short_name, stderr);
    fputs(": ", stderr);
    va_start(ap, message);
    vfprintf(stderr, message, ap);
    va_end(ap);
    if(errnum)
    {
        fputs(": ", stderr);
        fputs(strerror(errnum), stderr);
    }
    fputc('\n', stderr);
    if(status)
    {
        exit(status);
    }
}

#else

char _iso_c_forbids_an_empty_translation_unit;

#endif /* !defined(HAVE_ERROR) && !defined(_GNU_SOURCE) */
