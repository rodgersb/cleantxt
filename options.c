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

/** @file options.c
    Command-line options parsing routines for @c cleantxt. */

#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include <string.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifdef HAVE_ERROR_H
#   include <error.h>
#else
#   include <compat/error.h>
#endif

#ifndef HAVE_PROGRAM_INVOCATION_SHORT_NAME
#   include <compat/progname.h>
#endif

#ifdef HAVE_GETOPT_H
#   include <getopt.h>
#else
#   include <compat/goptlong.h>
#endif

#include "options.h"

/** Definition for boolean constant @e false */
#define FALSE 0

/** Definition for boolean constant @e true */
#define TRUE (!FALSE)

const char STDIN_FILE_NAME[] = "-";
const char STDOUT_FILE_NAME[] = "-";

/** Human-readable descriptions of each end-of-line mode */
static const char *const EOL_DESC_STR[] =
{
    "LF",
    "CR",
    "CR+LF"
};

/** Short option string to supply to @c getopt() when parsing the
    command-line arguments. */
static const char shortopts[] = "chlmo:RrsT:t:VZz";

/** Long option array to supply to @c getopt_long() when parsing the
    command-line arguments. */
static const struct option longopts[] =
{
    { "crlf", no_argument, NULL, 'c' },
    { "help", no_argument, NULL, 'h' },
    { "lf", no_argument, NULL, 'l' },
    { "cr", no_argument, NULL, 'm' },
    { "output", required_argument, NULL, 'o' },
    { "remove-ctrl-z", no_argument, NULL, 'R' },
    { "tabs", no_argument, NULL, 'r' },
    { "spaces", no_argument, NULL, 's' },
    { "tab-min", required_argument, NULL, 'T' },
    { "tab-size", required_argument, NULL, 't' },
    { "version", no_argument, NULL, 'V' },
    { "stop-at-ctrl-z", no_argument, NULL, 'Z' },
    { "add-ctrl-z", no_argument, NULL, 'z' },
    { NULL }
};

struct cleantxt_options options;

void print_help_message(void)
{
    /*             1111111111222222222233333333334444444444555555555566666666667777777777 */
    /*   01234567890123456789012345678901234567890123456789012345678901234567890123456789 */
    printf(
        "Usage: %s [OPTION]... [FILE]...\n"
        "   or: %s [OPTION]... -o OUTFILE INFILE\n"
        "Cleans up tab, space and end-of-line formatting in text files/streams.\n"
        "\n",
        program_invocation_short_name, program_invocation_short_name);
    printf(
        "  -c, --crlf            Use CR+LF for EOL seq. (default under DOS/MS-Windows)\n"
        "  -l, --lf              Use LF for EOL character (default under Unix)\n"
        "  -m, --cr              Use CR for EOL character\n"
        "  -o, --output=file     Write filtered output to given file.\n"
        "                        Only one input file may be given in this mode.\n");
    printf(
        "  -R, --remove-ctrl-z   Remove any ctrl-z characters encountered\n"
        "  -r, --tabs            Replace spaces with tab characters wherever possible\n"
        "  -s, --spaces          Expand tab characters into spaces (default action)\n"
        "  -T, --tab-min=n       Minimum whitespace gap for inserting tabs (default=%d)\n",
        DEFAULT_TAB_MIN);
    printf(
        "  -t, --tab-size=n      Interpret tab stops as n-columns wide (default=%d)\n"
        "  -Z, --stop-at-ctrl-z  Interpret ctrl-z characters as end-of-file\n"
        "  -z, --add-ctrl-z      Append a ctrl-z character at end-of-file\n"
        "\n",
        DEFAULT_TAB_SIZE);
    printf("Default end-of-line sequence on this platform: %s\n"
        "\n"
        "If no input file names are given, or `-' is specified as a file name,\n"
        "then standard input is filtered to standard output. All files are\n"
        "modified in-place unless the `-o' argument is given.\n",
        EOL_DESC_STR[DEFAULT_EOL_MODE]);
}

void parse_options(int argc, char *const *argv, jmp_buf *jmp_if_error)
{
    /* c: Stores the short-option character version of the current option */
    int c;

    /* Parse the command line options, one option per loop. Break out of
       the loop when no more options are found. If an error is
       encountered, or the help message is requested, then terminate the
       program. */
    do
    {
        c = getopt_long(argc, argv, shortopts, longopts, NULL);

        switch(c)
        {
            case 'c':
                /* Use DOS-style CR+LF for end-of-line sequence */
                options.eol_mode = EM_CRLF;
                break;
            case 'h':
                /* User wants to see the help message */
                options.program_mode = PM_SHOW_HELP;
                return;
            case 'l':
                /* Use UNIX-style LF for end-of-line sequence */
                options.eol_mode = EM_LF;
                break;
            case 'm':
                /* Use Macintosh-style CR for end-of-line sequence */
                options.eol_mode = EM_CR;
                break;
            case 'o':
                /* String argument contains output file */
                options.output_file_name = optarg;
                break;
            case 'R':
                /* User wants to silently discard any ctrl-Z characters */
                options.remove_ctrl_z = TRUE;
                break;
            case 'r':
                /* Fill whitespace gaps using tabs wherever possible */
                options.whitespace_mode = WM_TAB;
                break;
            case 's':
                /* Fill whitespace gaps using spaces */
                options.whitespace_mode = WM_SPACE;
                break;
            case 'T':
                /* Argument contains whitespace threshold for inserting tabs */
                options.tab_min = atoi(optarg);
                if(options.tab_min < 1)
                {
                    if(opterr)
                    {
                        error(0, 0, "Minimum whitespace gap must be a positive integer: %s", optarg);
                    }
                    longjmp(*jmp_if_error, TRUE);
                }
                break;
            case 't':
                /* Argument contains tab margin size */
                options.tab_size = atoi(optarg);
                if(options.tab_size < 1)
                {
                    if(opterr)
                    {
                        error(0, 0, "Tab size must be a positive integer: %s", optarg);
                    }
                    longjmp(*jmp_if_error, TRUE);
                }
                break;
            case 'V':
                /* User wants to see program version */
                options.program_mode = PM_SHOW_VERSION;
                return;
            case 'Z':
                /* A DOS EOF (Ctrl-Z) marker is considered end-of-file */
                options.stop_at_ctrl_z = TRUE;
                break;
            case 'z':
                /* Append a DOS EOF (Ctrl-Z) marker to the end of each file */
                options.add_ctrl_z = TRUE;
                break;
            case '?':
            case ':':
                /* Invalid/missing argument. The getopt() call will print
                   an error message describing to the user what the
                   problem is. Print an additional message informing the
                   user on how to get option help and terminate. */
                print_try_help_message();
                longjmp(*jmp_if_error, TRUE);
                break;
        }
    }
    while(c >= 0);

    /* Check if an output file name was supplied */
    if(options.output_file_name)
    {
        /* Check how many non-option arguments were supplied */
        if(optind + 1 == argc)
        {
            /* The user has specified an input file name that contains source material */
            options.input_file_name = argv[optind];
            options.program_mode = PM_PROCESS_STREAM;
        }
        else if(optind == argc)
        {
            /* The user hasn't provided an input file name. Use standard
               input as the input file. */
            options.input_file_name = STDIN_FILE_NAME;
            options.program_mode = PM_PROCESS_STREAM;
        }
        else
        {
            /* User specified multiple input files for the one output file  */
            if(opterr)
            {
                error(0, 0, "Only one input file may be specified if output file given");
            }
            longjmp(*jmp_if_error, TRUE);
        }
    }
    else if(optind < argc)
    {
        /* If no output file name was supplied, but at least one
           non-option argument is present, then the user has specified a
           list of files to process in-place. */
        options.file_name_list = (const char *const *)(argv + optind);
        options.program_mode = PM_PROCESS_FILE_LIST;
    }
    else
    {
        /* No input or output file names supplied. Use standard input
           for the input file and standard output for the output file. */
        options.input_file_name = STDIN_FILE_NAME;
        options.output_file_name = STDOUT_FILE_NAME;
        options.program_mode = PM_PROCESS_STREAM;
    }
}

void init_options(void)
{
    memset(&options, 0, sizeof(struct cleantxt_options));
    options.program_mode = PM_UNKNOWN;
    options.tab_size = DEFAULT_TAB_SIZE;
    options.tab_min = DEFAULT_TAB_MIN;
    options.whitespace_mode = DEFAULT_WHITESPACE_MODE;
    options.eol_mode = DEFAULT_EOL_MODE;
}

void print_try_help_message(void)
{
    if(opterr)
    {
        fprintf(stderr, "Try `%s --help' for more information.\n",
            program_invocation_short_name);
    }
}
