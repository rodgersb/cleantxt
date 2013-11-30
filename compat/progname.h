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

/** @file progname.h
    Allows non-GNU libc environments to determine the name of the
    program for help messages. */

#ifndef COMPAT_PROGNAME_H
#define COMPAT_PROGNAME_H

/** Name of the program. Used if the C library doesn't provide the
    global variable @c program_invocation_short_name. */
extern char *program_invocation_short_name;

/** Identifies the name of the program from @c argv[0] passed to the @c
    main() function, and stores a pointer to it in @a
    program_invocation_short_name. This function must be called at the
    very start of the program if not linked with GNU libc.

    @param argv0 This should be the element @c argv[0] that the @c
    main() function was called with. It is assumed to point to an area
    of memory that will remain valid for the entire lifespan of the
    process. */
extern void init_program_invocation_short_name(char *argv0);

#endif /* !COMPAT_PROGNAME_H */
