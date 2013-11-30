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

/** @file main.c
    Top-level entry-point module for cleantxt.

    This utility filters through text files/streams, expanding tabs into
    spaces (or vice versa), trims trailing spaces off the ends of lines
    and converts between different types of end-of-line characters, to
    make portability of text files between DOS, UNIX & Macintosh a lot
    easier, and bridging the division caused by coding style politics
    that sometimes pervades in software development teams.

    It also fixes the common problem of not having a newline character
    on the last line of a file, which can be problematic to some Unix
    programs/utilities, and may create warnings with ANSI C compilers.

    This source code is intended to be ANSI C90 compliant. */

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <stdlib.h>
#include <setjmp.h>
#include <stdio.h>
#include <assert.h>

#ifdef HAVE_PROGRAM_INVOCATION_SHORT_NAME
#   include <errno.h>
#else
#   include <compat/progname.h>
#endif

#include "options.h"
#include "procfile.h"

/** Program entry point.

    @param argc Number of command-line arguments, including program name.
    @param argv String array of command-line arguments, including
    program name.
    @returns Program exit status code. */
int main(int argc, char *const *argv)
{
    /* jmp_on_error: Records state information for an exception jump to
       be made if an error occurs. */
    jmp_buf jmp_on_error;

#   ifndef HAVE_PROGRAM_INVOCATION_SHORT_NAME
        init_program_invocation_short_name(argv[0]);
#   endif
    init_options();

    if(setjmp(jmp_on_error))
    {
        /* Execution will branch here if an error occurs */
        return EXIT_FAILURE;
    }

    parse_options(argc, argv, &jmp_on_error);
    switch(options.program_mode)
    {
        case PM_SHOW_HELP:
            print_help_message();
            break;
        case PM_SHOW_VERSION:
            puts(VERSION);
            break;
        case PM_PROCESS_STREAM:
            /* User specified at most one input file and one output file.
               Filter the input file and write the contents to the output
               file. */
            process_file(options.input_file_name, options.output_file_name, &jmp_on_error);
            break;
        case PM_PROCESS_FILE_LIST:
            /* Process each input file in-place */
            process_file_list(options.file_name_list, &jmp_on_error);
            break;
        default:
            /* Execution should not reach here */
            assert(0);
    }
    return EXIT_SUCCESS;
}
