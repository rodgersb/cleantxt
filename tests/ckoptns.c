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

/** @file tests/ckoptns.c
    Test suite for options module. This is a safeguard against
    accidentally breaking the command-line arguments parsing logic. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <check.h>

#ifdef HAVE_GETOPT_H
#   include <getopt.h>
#else
#   include <compat/goptlong.h>
#endif

#include "../options.h"

#ifndef __GNUC__
#   define __attribute__(x) /* Intentionally left blank */
#endif

/** Definition for boolean constant @e false */
#define FALSE 0

/** Definition for boolean constant @e true */
#define TRUE (!FALSE)

/** Stores the argv vector for the most recent invocation of
    #try_options. This structure needs to persist beyond the end of
    #try_options since the @a file_name_list pointer in the #options
    structure attempts to point to part of it. Will be freed and
    re-allocated on subsequent invocations. */
static char **try_argv = NULL;

/** Invokes #parse_options by simulating a command line.

    The #options structure will be initialised via #init_options
    beforehand. @c getopt_long() error messages will be suppressed (so
    they don't interfere with the test result output).

    @param arg1 First argument on the command line; successive arguments
    follow. Each argument should be given as a pointer to a
    null-terminated string. The list <strong>must</strong> be terminated
    with a @c NULL pointer sentinel otherwise a segfault may occur.
    @return @c TRUE if #parse_options successfully parsed the arguments;
    @c FALSE otherwise. */
static int try_options(char *arg1, ...)
    __attribute__((sentinel));
/* note: GCC's attribute(sentinel) enables producing a compiler warning if
   the final argument in any invocation of that function is not NULL. */

static int try_options(char *arg1, ...)
{
    /* argc: Number of arguments counted (plus program name) */
    /* on_cmdline_error: Execution jumps here if a parsing error is encountered */
    int argc = 1;
    jmp_buf on_cmdline_error;

    optind = 1; /* Reset getopt's state in case invoked multiple times */
    opterr = 0; /* Quell getopt() error messages */

    /* Collect arguments from the stack and build up a string vector
       that looks like the argc/argv one main() receives. */
    if(arg1)
    {
        /* ap: Points to current argument on the arguments list */
        va_list ap;

        argc++;
        va_start(ap, arg1);
        while(va_arg(ap, char *) != NULL)
        {
            argc++;
        }
        va_end(ap);
    }
    if(try_argv)
    {
        free(try_argv);
    }
    try_argv = malloc((argc + 1) * sizeof(char *));
    try_argv[0] = "ckoptns";
    try_argv[1] = arg1;
    if(arg1)
    {
        /* i: Argument index used when collecting arguments on second pass */
        /* ap: Points to current argument on the arguments list */
        int i;
        va_list ap;

        va_start(ap, arg1);
        for(i = 2; i < argc; i++)
        {
            try_argv[i] = va_arg(ap, char *);
        }
        va_end(ap);
        try_argv[argc] = NULL;
    }

    init_options();
    if(setjmp(on_cmdline_error))
    {
        /* Execution will branch here if a parsing error is encountered */
        return FALSE;
    }
    parse_options(argc, try_argv, &on_cmdline_error);
    return TRUE;
}

static void assert_dfl_whitespace_mode(void)
    { ck_assert(options.whitespace_mode == DEFAULT_WHITESPACE_MODE); }
static void assert_dfl_eol_mode(void)
    { ck_assert(options.eol_mode == DEFAULT_EOL_MODE); }
static void assert_dfl_tab_size(void)
    { ck_assert(options.tab_size == DEFAULT_TAB_SIZE); }
static void assert_dfl_tab_min(void)
    { ck_assert(options.tab_min == DEFAULT_TAB_MIN); }
static void assert_dont_stop_at_ctrl_z(void)
    { ck_assert(!options.stop_at_ctrl_z); }
static void assert_dont_add_ctrl_z(void)
    { ck_assert(!options.add_ctrl_z); }

START_TEST(no_options)
{
    /* Need two NULLs to keep GCC happy */
    ck_assert(try_options(NULL, NULL));
    ck_assert(options.program_mode == PM_PROCESS_STREAM);
    ck_assert(strcmp(options.input_file_name, STDIN_FILE_NAME) == 0);
    ck_assert(strcmp(options.output_file_name, STDOUT_FILE_NAME) == 0);
    assert_dfl_whitespace_mode();
    assert_dfl_eol_mode();
    assert_dfl_tab_size();
    assert_dfl_tab_min();
    assert_dont_stop_at_ctrl_z();
    assert_dont_add_ctrl_z();
}
END_TEST

START_TEST(help_message)
{
    ck_assert(try_options("-h", NULL));
    ck_assert(options.program_mode == PM_SHOW_HELP);
    ck_assert(try_options("--help", "null", NULL));
    ck_assert(options.program_mode == PM_SHOW_HELP);
}
END_TEST

START_TEST(version_number)
{
    ck_assert(try_options("-V", NULL));
    ck_assert(options.program_mode == PM_SHOW_VERSION);
    ck_assert(try_options("--version", "foo", NULL));
    ck_assert(options.program_mode == PM_SHOW_VERSION);
}
END_TEST

START_TEST(single_file)
{
    static char FILE[] = "baz";

    ck_assert(try_options(FILE, NULL));
    ck_assert(options.program_mode == PM_PROCESS_FILE_LIST);
    ck_assert(options.file_name_list != NULL);
    ck_assert(strcmp(FILE, options.file_name_list[0]) == 0);
    ck_assert(options.file_name_list[1] == NULL);
    assert_dfl_whitespace_mode();
    assert_dfl_eol_mode();
    assert_dfl_tab_size();
    assert_dfl_tab_min();
    assert_dont_stop_at_ctrl_z();
    assert_dont_add_ctrl_z();
}
END_TEST

START_TEST(batch_list)
{
    static char FILE1[] = "foo";
    static char FILE2[] = "bar";
    static char FILE3[] = "quux";

    ck_assert(try_options(FILE1, FILE2, FILE3, STDIN_FILE_NAME, NULL));
    ck_assert(options.program_mode == PM_PROCESS_FILE_LIST);
    ck_assert(options.file_name_list != NULL);
    ck_assert(strcmp(FILE1, options.file_name_list[0]) == 0);
    ck_assert(strcmp(FILE2, options.file_name_list[1]) == 0);
    ck_assert(strcmp(FILE3, options.file_name_list[2]) == 0);
    ck_assert(strcmp(STDIN_FILE_NAME, options.file_name_list[3]) == 0);
    ck_assert(options.file_name_list[4] == NULL);
    assert_dfl_whitespace_mode();
    assert_dfl_eol_mode();
    assert_dfl_tab_size();
    assert_dfl_tab_min();
    assert_dont_stop_at_ctrl_z();
    assert_dont_add_ctrl_z();
}
END_TEST

START_TEST(source_dest_file)
{
    static char INFILE[] = "foo";
    static char OUTFILE[] = "bar";
    ck_assert(try_options("-o", OUTFILE, INFILE, NULL));
    ck_assert(options.program_mode == PM_PROCESS_STREAM);
    ck_assert(strcmp(options.input_file_name, INFILE) == 0);
    ck_assert(strcmp(options.output_file_name, OUTFILE) == 0);
    assert_dfl_whitespace_mode();
    assert_dfl_eol_mode();
    assert_dfl_tab_size();
    assert_dfl_tab_min();
    assert_dont_stop_at_ctrl_z();
    assert_dont_add_ctrl_z();
}
END_TEST

START_TEST(too_many_files_in_output_mode)
{
    static char OUTFILE[] = "zap";
    static char INFILE1[] = "foo";
    static char INFILE2[] = "bar";

    ck_assert(!try_options("-o", OUTFILE, INFILE1, INFILE2, NULL));
}
END_TEST

START_TEST(test_eol_modes)
{
    ck_assert(try_options("-m", NULL));
    ck_assert(options.eol_mode == EM_CR);
    ck_assert(try_options("--cr", NULL));
    ck_assert(options.eol_mode == EM_CR);
    assert_dfl_whitespace_mode();
    assert_dfl_tab_size();
    assert_dfl_tab_min();
    assert_dont_stop_at_ctrl_z();
    assert_dont_add_ctrl_z();

    ck_assert(try_options("-l", NULL));
    ck_assert(options.eol_mode == EM_LF);
    ck_assert(try_options("--lf", NULL));
    ck_assert(options.eol_mode == EM_LF);
    assert_dfl_whitespace_mode();
    assert_dfl_tab_size();
    assert_dfl_tab_min();
    assert_dont_stop_at_ctrl_z();
    assert_dont_add_ctrl_z();

    ck_assert(try_options("-c", NULL));
    ck_assert(options.eol_mode == EM_CRLF);
    ck_assert(try_options("--crlf", NULL));
    ck_assert(options.eol_mode == EM_CRLF);
    assert_dfl_whitespace_mode();
    assert_dfl_tab_size();
    assert_dfl_tab_min();
    assert_dont_stop_at_ctrl_z();
    assert_dont_add_ctrl_z();
}
END_TEST

START_TEST(test_whitespace_modes)
{
    ck_assert(try_options("-r", NULL));
    ck_assert(options.whitespace_mode == WM_TAB);
    ck_assert(try_options("--tabs", NULL));
    ck_assert(options.whitespace_mode == WM_TAB);
    assert_dfl_eol_mode();
    assert_dfl_tab_size();
    assert_dfl_tab_min();
    assert_dont_stop_at_ctrl_z();
    assert_dont_add_ctrl_z();

    ck_assert(try_options("-s", NULL));
    ck_assert(options.whitespace_mode == WM_SPACE);
    ck_assert(try_options("--spaces", NULL));
    ck_assert(options.whitespace_mode == WM_SPACE);
    assert_dfl_eol_mode();
    assert_dfl_tab_size();
    assert_dfl_tab_min();
    assert_dont_stop_at_ctrl_z();
    assert_dont_add_ctrl_z();
}
END_TEST

START_TEST(test_ctrl_z_modes)
{
    ck_assert(try_options("-z", NULL));
    ck_assert(options.add_ctrl_z);
    ck_assert(try_options("--add-ctrl-z", NULL));
    ck_assert(options.add_ctrl_z);
    assert_dont_stop_at_ctrl_z();
    assert_dfl_whitespace_mode();
    assert_dfl_eol_mode();
    assert_dfl_tab_size();
    assert_dfl_tab_min();

    ck_assert(try_options("-Z", NULL));
    ck_assert(options.stop_at_ctrl_z);
    ck_assert(try_options("--stop-at-ctrl-z", NULL));
    ck_assert(options.stop_at_ctrl_z);
    assert_dfl_whitespace_mode();
    assert_dfl_eol_mode();
    assert_dfl_tab_size();
    assert_dfl_tab_min();
    assert_dont_add_ctrl_z();

}
END_TEST

START_TEST(test_tab_sizes)
{
    ck_assert(!try_options("-t", NULL));
    ck_assert(!try_options("-t", "foo", NULL));
    ck_assert(!try_options("-t", "0", NULL));
    ck_assert(!try_options("-t", "-1", NULL));
    ck_assert(try_options("-t7", NULL));
    ck_assert(options.tab_size == 7);
    ck_assert(try_options("--tab-size", "5", NULL));
    ck_assert(options.tab_size == 5);
    assert_dfl_whitespace_mode();
    assert_dfl_eol_mode();
    assert_dfl_tab_min();
    assert_dont_stop_at_ctrl_z();
    assert_dont_add_ctrl_z();
}
END_TEST

START_TEST(test_tab_min)
{
    ck_assert(!try_options("-T", NULL));
    ck_assert(!try_options("-T", "bar", NULL));
    ck_assert(!try_options("-T", "0", NULL));
    ck_assert(!try_options("-T", "-2", NULL));
    ck_assert(try_options("-T9", NULL));
    ck_assert(options.tab_min == 9);
    ck_assert(try_options("--tab-min", "3", NULL));
    ck_assert(options.tab_min == 3);
    assert_dfl_whitespace_mode();
    assert_dfl_eol_mode();
    assert_dfl_tab_size();
    assert_dont_stop_at_ctrl_z();
    assert_dont_add_ctrl_z();
}
END_TEST

START_TEST(test_invalid_option)
{
    ck_assert(!try_options("-@", NULL));
    ck_assert(!try_options("--this-is-not-a-valid-option", NULL));
}
END_TEST

Suite *init_suite(void)
{
    Suite *s = suite_create("options");
    TCase *tc_core = tcase_create("core");
    tcase_add_test(tc_core, no_options);
    tcase_add_test(tc_core, help_message);
    tcase_add_test(tc_core, version_number);
    tcase_add_test(tc_core, single_file);
    tcase_add_test(tc_core, batch_list);
    tcase_add_test(tc_core, source_dest_file);
    tcase_add_test(tc_core, too_many_files_in_output_mode);
    tcase_add_test(tc_core, test_eol_modes);
    tcase_add_test(tc_core, test_whitespace_modes);
    tcase_add_test(tc_core, test_ctrl_z_modes);
    tcase_add_test(tc_core, test_tab_sizes);
    tcase_add_test(tc_core, test_tab_min);
    tcase_add_test(tc_core, test_invalid_option);
    suite_add_tcase(s, tc_core);
    return s;
}
