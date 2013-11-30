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

/** @file progname.c
    Allows non-GNUlibc environments to determine the name of the
    program for help messages. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#if !defined(HAVE_PROGRAM_INVOCATION_SHORT_NAME) && !defined(_GNU_SOURCE)

#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* DJGPP has basename() here instead */

#ifdef HAVE_LIBGEN_H
#include <libgen.h> /* for basename() on BSD-type systems */
#endif /* HAVE_LIBGEN_H */

#include "progname.h"

char *program_invocation_short_name;

void init_program_invocation_short_name(char *argv0)
{
    /* argv0_len: Character length of argv[0] element string, not incl. terminator.  */
    /* argv0_tmp: Temporary copy of argv[0], as basename() may modify its argument. */
    /* basename_str: Return value from basename() function */
    /* basename_len: Length of basename part of argv[0] */
    size_t argv0_len = strlen(argv0);
    char *argv0_tmp = strdup(argv0);
    char *basename_str;
    size_t basename_len;

    basename_str = basename(argv0_tmp);
    basename_len = strlen(basename_str);
    program_invocation_short_name = argv0 + (argv0_len - basename_len);
    free(argv0_tmp);
}

#else

char _iso_c_forbids_an_empty_translation_unit;

#endif /* !defined(HAVE_PROGRAM_INVOCATION_SHORT_NAME) && !defined(_GNU_SOURCE) */
