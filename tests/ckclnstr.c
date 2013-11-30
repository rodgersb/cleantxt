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

/** @file tests/ckclnstr.c
    Test suite for cleanstr module. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <check.h>

#include "../cleanstr.h"
#include "../options.h"
#include "helpers/io.h"

#ifndef __GNUC__
#   define __attribute__(x) /* Intentionally left blank */
#endif

/* Simplifies the type expressions needed */
typedef char ** argv_t;

static void build_argv(
    char *arg1,
    va_list _ap,
    int *argc,
    volatile argv_t *argv)
{
    /* Collect arguments from the stack and build up a string vector
       that looks like the argc/argv one main() receives. */
    if(arg1)
    {
        /* ap: Points to current argument on the arguments list */
        va_list ap = _ap;

        (*argc)++;
        while(va_arg(ap, char *) != NULL)
        {
            (*argc)++;
        }
    }
    *argv = malloc((*argc + 1) * sizeof(char *));
    (*argv)[0] = "ckclnstr";
    (*argv)[1] = arg1;
    if(arg1)
    {
        /* i: Argument index used when collecting arguments on second pass */
        /* ap: Points to current argument on the arguments list */
        int i;
        va_list ap = _ap;

        for(i = 2; i < *argc; i++)
        {
            (*argv)[i] = va_arg(ap, char *);
        }
        (*argv)[*argc] = NULL;
    }
}

static clean_stream_result_t try_cleanstr(
    const char *input_str,
    const char *expect_str,
    char *arg1,
    ...) __attribute__((sentinel));
/* note: GCC's attribute(sentinel) enables producing a compiler warning if
   the final argument in any invocation of that function is not NULL. */

static clean_stream_result_t try_cleanstr(
    const char *input_str,
    const char *expect_str,
    char *arg1,
    ...)
{
    /* argc: Number of arguments counted (plus program name) */
    /* argv: Command-line arguments vector, similar to the one main() receives */
    /* on_cmdline_error: Execution jumps here if a parsing error is encountered */
    /* on_io_error: Execution jumps here if an I/O error is encountered */
    /* input_file: File prepared with input content */
    /* actual_file: Temporary file that will contain actual output */
    /* res: Return result of clean_stream() */
    /* ap: Points to current argument on the arguments list */
    int argc = 1;
    /* The volatile requirement is needed to force GCC's -O2 optimiser
       to make argv an automatic (stack) rather than register variable,
       to supress it from complaining about longjmp() clobbering it
       when using -O2 -Werror. */
    volatile argv_t argv;
    jmp_buf on_cmdline_error;
    jmp_buf on_io_error;
    FILE *input_file;
    FILE *actual_file;
    clean_stream_result_t res;
    va_list ap;

    optind = 1; /* Reset getopt's state in case invoked multiple times */

    va_start(ap, arg1);
    build_argv(arg1, ap, &argc, &argv);
    va_end(ap);

    init_options();
    if(setjmp(on_cmdline_error))
    {
        /* Execution will branch here if a parsing error is encountered */
        ck_abort_msg("The command-line options supplied contain an error.");
    }
    parse_options(argc, argv, &on_cmdline_error);
    free(argv);

    input_file = create_input_file_from_str(input_str);
    actual_file = tmpfile();
    ck_assert_msg(actual_file != NULL,
        "Unable to create temporary output file: %s",
        strerror(errno));

    if(setjmp(on_io_error))
    {
        /* Execution will branch here if an I/O error is encountered */
        ck_abort_msg("I/O error encountered: %s", strerror(errno));
    }
    res = clean_stream(input_file, actual_file, &on_io_error);
    assert_output_file_contents_match_str(expect_str, actual_file);
    ck_assert(fclose(actual_file) == 0);
    ck_assert(fclose(input_file) == 0);
    return res;
}

#if 0
/* Copy this template to create a new test: */
    ck_assert(try_cleanstr(
        /* Input */
        "",
        /* Output */
        "",
        /* Arguments */
        "-lst8", NULL) == CSR_STREAM_MODIFIED);
#endif

START_TEST(no_change)
{
    ck_assert(try_cleanstr(
        /* Input */
        "Hello World!\n",
        /* Output */
        "Hello World!\n",
        /* Arguments */
        "-lst8", NULL) == CSR_STREAM_UNMODIFIED);
}
END_TEST

START_TEST(empty_input)
{
    ck_assert(try_cleanstr(
        /* Input */
        "",
        /* Output */
        "",
        /* Arguments */
        "-lst8", NULL) == CSR_STREAM_UNMODIFIED);
}
END_TEST

START_TEST(single_line_without_newline)
{
    ck_assert(try_cleanstr(
        /* Input */
        "hello",
        /* Output */
        "hello\n",
        /* Arguments */
        "-lst8", NULL) == CSR_STREAM_MODIFIED);
}
END_TEST

START_TEST(redundant_trailing_newlines)
{
    ck_assert(try_cleanstr(
        /* Input */
        "blah\n\n\nblah\n\n\n",
        /* Output */
        "blah\n\n\nblah\n",
        /* Arguments */
        "-lst8", NULL) == CSR_STREAM_MODIFIED);
}
END_TEST

START_TEST(expand_tabs)
{
    ck_assert(try_cleanstr(
        /* Input */
        "Veni\n"
        "\tVidi\n"
        "\t\tVici\n",
        /* Output */
        "Veni\n"
        "    Vidi\n"
        "        Vici\n",
        /* Arguments */
        "-lst4", NULL) == CSR_STREAM_MODIFIED);
}
END_TEST

START_TEST(collapse_spaces)
{
    ck_assert(try_cleanstr(
        /* Input */
        "Quidquid\n"
        "    Latine\n"
        "        dictum\n"
        "            sit\n"
        "               altum\n"
        "                    videtur\n",
        /* Output */
        "Quidquid\n"
        "\tLatine\n"
        "\t\tdictum\n"
        "\t\t\tsit\n"
        "\t\t\t   altum\n"
        "\t\t\t\t\tvidetur\n",
        /* Arguments */
        "-lrt4", NULL) == CSR_STREAM_MODIFIED);
}
END_TEST

START_TEST(convert_crlf_to_lf)
{
    ck_assert(try_cleanstr(
        /* Input (note first line only has LF, rest have CR+LF) */
        "Unix came first,\n"
        "MS-DOS and\r\n"
        "Microsoft Windows\r\n"
        "came later.\r\n",
        /* Output */
        "Unix came first,\n"
        "MS-DOS and\n"
        "Microsoft Windows\n"
        "came later.\n",
        /* Arguments */
        "-lst8", NULL) == CSR_STREAM_MODIFIED);
}
END_TEST

START_TEST(convert_lf_to_crlf)
{
    ck_assert(try_cleanstr(
        /* Input */
        "COUNTRY=061,437,SYSTEM\\DOS\\COUNTRY.SYS\n"
        "FILES=20\n"
        "BUFFERS=40\n"
        "DOS=HIGH,UMB\n",
        /* Output */
        "COUNTRY=061,437,SYSTEM\\DOS\\COUNTRY.SYS\r\n"
        "FILES=20\r\n"
        "BUFFERS=40\r\n"
        "DOS=HIGH,UMB\r\n",
        /* Arguments */
        "-cst8", NULL) == CSR_STREAM_MODIFIED);
}
END_TEST

START_TEST(convert_lf_to_cr)
{
    ck_assert(try_cleanstr(
        /* Input */
        "Classic Mac OS users all rejoice!\n"
        "Click your second mouse button!\n",
        /* Output */
        "Classic Mac OS users all rejoice!\r"
        "Click your second mouse button!\r",
        /* Arguments */
        "-mst8", NULL) == CSR_STREAM_MODIFIED);
}
END_TEST

START_TEST(eat_trailing_spaces)
{
    ck_assert(try_cleanstr(
        /* Input */
        "I   \n"
        "See   \t   \t   \n"
        "No         \n"
        "    Trailing   \n"
        "\tSpaces         \n",
        /* Output */
        "I\n"
        "See\n"
        "No\n"
        "    Trailing\n"
        "        Spaces\n",
        /* Arguments */
        "-lst8", NULL) == CSR_STREAM_MODIFIED);
}
END_TEST

START_TEST(tab_min_spaces)
{
    ck_assert(try_cleanstr(
        /* Input */
        "01234567" "89abcde\n"
        "0123456\t""89abcde\n"
        "012345\t" "89abcde\n"
        "01234\t"  "89abcde\n"
        "0123\t"   "89abcde\n"
        "012\t"    "89abcde\n"
        "01\t"     "89abcde\n"
        "0\t"      "89abcde\n"
        "\t"       "89abcde\n",
        /* Output */
        "0123456789abcde\n"
        "0123456 89abcde\n"
        "012345  89abcde\n"
        "01234   89abcde\n"
        "0123    89abcde\n"
        "012     89abcde\n"
        "01      89abcde\n"
        "0       89abcde\n"
        "        89abcde\n",
        /* Arguments */
        "-lst8", "-T3", NULL) == CSR_STREAM_MODIFIED);
}
END_TEST

START_TEST(tab_min_tabs)
{
    ck_assert(try_cleanstr(
        /* Input */
        "01234567" "89abcde\n"
        "0123456\t""89abcde\n"
        "012345\t" "89abcde\n"
        "01234\t"  "89abcde\n"
        "0123\t"   "89abcde\n"
        "012\t"    "89abcde\n"
        "01\t"     "89abcde\n"
        "0\t"      "89abcde\n"
        "\t"       "89abcde\n",
        /* Output */
        "01234567""89abcde\n"
        "0123456 ""89abcde\n"
        "012345  ""89abcde\n"
        "01234\t" "89abcde\n"
        "0123\t"  "89abcde\n"
        "012\t"   "89abcde\n"
        "01\t"    "89abcde\n"
        "0\t"     "89abcde\n"
        "\t"      "89abcde\n",
        /* Arguments */
        "-lrt8", "-T3", NULL) == CSR_STREAM_MODIFIED);
}
END_TEST

START_TEST(stop_at_ctrl_z)
{
    ck_assert(try_cleanstr(
        /* Input */
        "Hello\n\032World!\n",
        /* Output */
        "Hello\n\032",
        /* Arguments */
        "-Zlst8", NULL) == CSR_STREAM_MODIFIED);
        /* See comments in cleanstr.c for reasons why
           CSR_STREAM_MODIFIED is returned despite no modifications
           prior to the ctrl-Z character. */
}
END_TEST

START_TEST(add_ctrl_z)
{
    ck_assert(try_cleanstr(
        /* Input */
        "Hello World!",
        /* Output */
        "Hello World!\n\032",
        /* Arguments */
        "-zlst8", NULL) == CSR_STREAM_MODIFIED);

}
END_TEST

START_TEST(add_ctrl_z_only_if_not_last_non_ws_char)
{
    ck_assert(try_cleanstr(
        /* Input */
        "Hello World!\n\032",
        /* Output */
        "Hello World!\n\032\n",
        /* Arguments */
        "-zlst8", NULL) == CSR_STREAM_MODIFIED);
}
END_TEST

START_TEST(remove_ctrl_z)
{
    ck_assert(try_cleanstr(
        /* Input */
        "Hello\032World\032!\n",
        /* Output */
        "HelloWorld!\n",
        /* Arguments */
        "-Rlst8", NULL) == CSR_STREAM_MODIFIED);
}
END_TEST

START_TEST(relocate_ctrl_z)
{
    ck_assert(try_cleanstr(
        /* Input */
        "Hello\032World!\n",
        /* Output */
        "HelloWorld!\n\032",
        /* Arguments */
        "-Rzlst8", NULL) == CSR_STREAM_MODIFIED);
}
END_TEST

START_TEST(dont_add_spurious_ctrl_z)
{
    ck_assert(try_cleanstr(
        /* Input */
        "Hello World!\032",
        /* Output */
        /* Note that a newline is also added before the ctrl-Z */
        "Hello World!\n\032",
        /* Arguments */
        "-Zzlst8", NULL) == CSR_STREAM_MODIFIED);
}
END_TEST

START_TEST(destroy_a_whitespace_program)
{
    ck_assert(try_cleanstr(
        /* Input */
        /* This is the Whitespace language "Hello World" program from
           http://en.wikipedia.org/wiki/Whitespace_%28programming_language%29 */
        "   \t  \t   \n"
        "\t\n"
        "     \t\t  \t \t\n"
        "\t\n"
        "     \t\t \t\t  \n"
        "\t\n"
        "     \t\t \t\t  \n"
        "\t\n"
        "     \t\t \t\t\t\t\n"
        "\t\n"
        "     \t \t\t  \n"
        "\t\n"
        "     \t     \n"
        "\t\n"
        "     \t\t\t \t\t\t\n"
        "\t\n"
        "     \t\t \t\t\t\t\n"
        "\t\n"
        "     \t\t\t  \t \n"
        "\t\n"
        "     \t\t \t\t  \n"
        "\t\n"
        "     \t\t  \t  \n"
        "\t\n"
        "     \t    \t\n"
        "\t\n"
        "  \n"
        "\n"
        "\n",
        /* Output */
        /* Should be completely empty */
        "",
        /* Arguments */
        "-lst8", NULL) == CSR_STREAM_MODIFIED);
}
END_TEST

Suite *init_suite(void)
{
    Suite *s = suite_create("cleanstr");
    TCase *tc_core = tcase_create("core");
    tcase_add_test(tc_core, no_change);
    tcase_add_test(tc_core, empty_input);
    tcase_add_test(tc_core, single_line_without_newline);
    tcase_add_test(tc_core, redundant_trailing_newlines);
    tcase_add_test(tc_core, expand_tabs);
    tcase_add_test(tc_core, collapse_spaces);
    tcase_add_test(tc_core, convert_crlf_to_lf);
    tcase_add_test(tc_core, convert_lf_to_crlf);
    tcase_add_test(tc_core, convert_lf_to_cr);
    tcase_add_test(tc_core, eat_trailing_spaces);
    tcase_add_test(tc_core, tab_min_spaces);
    tcase_add_test(tc_core, tab_min_tabs);
    tcase_add_test(tc_core, stop_at_ctrl_z);
    tcase_add_test(tc_core, add_ctrl_z);
    tcase_add_test(tc_core, add_ctrl_z_only_if_not_last_non_ws_char);
    tcase_add_test(tc_core, remove_ctrl_z);
    tcase_add_test(tc_core, relocate_ctrl_z);
    tcase_add_test(tc_core, dont_add_spurious_ctrl_z);
    tcase_add_test(tc_core, destroy_a_whitespace_program);
    suite_add_tcase(s, tc_core);
    return s;
}
