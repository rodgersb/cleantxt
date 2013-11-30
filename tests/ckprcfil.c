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

/** @file tests/ckprcfil.c
    Test suite for procfile module. */

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
#include <check.h>

#include "../procfile.h"
#include "../options.h"
#include "helpers/io.h"

/** Temporary filename template; this must be copied, not used directly
    with the @c mkstemp() library call, since it modifies its string
    argument in-place. */
static const char MKSTEMP_TEMPLATE[] = "tmXXXXXX";

START_TEST(test_process_file)
{
    static const char ORG_DATA[] = "\tHello world!\n";
    static const char EXP_DATA[] =  "    Hello world!\n";
    char *in_file_name = strdup(MKSTEMP_TEMPLATE);
    char *out_file_name = strdup(MKSTEMP_TEMPLATE);
    int in_fd = mkstemp(in_file_name);
    int out_fd = mkstemp(out_file_name);
    size_t org_data_len = strlen(ORG_DATA);
    jmp_buf on_io_error;
    FILE *actual_file;

    ck_assert(in_fd >= 0);
    ck_assert(out_fd >= 0);
    ck_assert(write(in_fd, ORG_DATA, org_data_len) == (ssize_t)org_data_len);
    ck_assert(close(in_fd) == 0);
    ck_assert(close(out_fd) == 0);

    init_options();
    options.tab_size = 4;
    options.tab_min = 1;
    options.whitespace_mode = WM_SPACE;
    options.eol_mode = EM_LF;

    if(setjmp(on_io_error))
    {
        /* Execution will branch here on I/O error */
        ck_abort_msg("I/O error occurred: %s", strerror(errno));
    }
    process_file(in_file_name, out_file_name, &on_io_error);
    actual_file = fopen(out_file_name, "rb");
    ck_assert(actual_file != NULL);
    assert_output_file_contents_match_str(EXP_DATA, actual_file);
    ck_assert(fclose(actual_file) == 0);
    ck_assert(unlink(in_file_name) == 0);
    ck_assert(unlink(out_file_name) == 0);
    free(in_file_name);
    free(out_file_name);
}
END_TEST

/** Number of input files for #test_process_file_list */
#define LIST_LEN 3

START_TEST(test_process_file_list)
{
    static const char *const ORG_DATA[LIST_LEN] =
        { "\tUnus\n",    "\tDuo\n",   "\tTres\n" };
    static const char *const EXP_DATA[LIST_LEN] =
        {  "    Unus\n",  "    Duo\n", "    Tres\n" };
    int fd[LIST_LEN];
    char *file_names[LIST_LEN + 1]; /* Must be null-terminated vector */
    int i;
    jmp_buf on_io_error;

    for(i = 0; i < LIST_LEN; i++)
    {
        size_t org_data_len = strlen(ORG_DATA[i]);
        ssize_t res;

        file_names[i] = strdup(MKSTEMP_TEMPLATE);
        fd[i] = mkstemp(file_names[i]);
        ck_assert_msg(fd[i] >= 0, "Unable to create temp file `%s': %s",
            file_names[i], strerror(errno));
        res = write(fd[i], ORG_DATA[i], org_data_len);
        ck_assert_msg(res == (ssize_t)org_data_len,
            "Unable to write to temp file `%s': %s",
            file_names[i], strerror(errno));
        res = close(fd[i]);
        ck_assert_msg(res == 0, "Unable to close temp file `%s': %s",
            file_names[i], strerror(errno));
    }
    file_names[LIST_LEN] = NULL;

    init_options();
    options.tab_size = 4;
    options.tab_min = 1;
    options.whitespace_mode = WM_SPACE;
    options.eol_mode = EM_LF;

    if(setjmp(on_io_error))
    {
        /* Execution will branch here on I/O error */
        ck_abort_msg("I/O error occurred: %s", strerror(errno));
    }
    process_file_list((const char *const *)file_names, &on_io_error);
    for(i = 0; i < LIST_LEN; i++)
    {
        FILE *actual_file;
        int res;

        actual_file = fopen(file_names[i], "rb");
        ck_assert_msg(actual_file != NULL,
            "Unable to open temp file `%s' for verification: %s",
            file_names[i], strerror(errno));
        assert_output_file_contents_match_str(EXP_DATA[i], actual_file);
        res = fclose(actual_file);
        ck_assert_msg(res == 0, "Unable to close temp file `%s': %s",
            file_names[i], strerror(errno));
        res = unlink(file_names[i]);
        ck_assert_msg(res == 0, "Unable to delete temp file `%s': %s",
            file_names[i], strerror(errno));
        free(file_names[i]);
    }
}
END_TEST

Suite *init_suite(void)
{
    Suite *s = suite_create("procfile");
    TCase *tc_core = tcase_create("core");
    tcase_add_test(tc_core, test_process_file);
    tcase_add_test(tc_core, test_process_file_list);
    suite_add_tcase(s, tc_core);
    return s;
}
