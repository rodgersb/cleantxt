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

/** @file tests/ckflmgmt.c
    Test suite for filemgmt module. */

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

#include "../filemgmt.h"

/** Temporary filename template; this must be copied, not used directly
    with the @c mkstemp() library call. */
static const char MKSTEMP_TEMPLATE[] = "tmXXXXXX";

/** Redirect stderr to this file during testing to suppress error
    messages from intefering with the test results output. Note that
    DJGPP under DOS will also emulate "/dev/null" as needed. */
static const char STDERR_SINK[] = "/dev/null";

static void setup(void)
{
    freopen(STDERR_SINK, "w", stderr);
}

static void teardown(void)
{
    fclose(stderr);
    stderr = fdopen(STDERR_FILENO, "w");
}

START_TEST(test_open_file__success)
{
    jmp_buf on_io_error;
    char *f_name = strdup(MKSTEMP_TEMPLATE);
    int f_fd = mkstemp(f_name);
    FILE *f;

    ck_assert(f_fd >= 0);
    ck_assert(close(f_fd) == 0);
    if(setjmp(on_io_error))
    {
        /* Execution will branch here upon an I/O error */
        int save_errno = errno;
        unlink(f_name);
        ck_abort_msg("I/O error occurred: %s", strerror(save_errno));
    }

    open_file(f_name, "rb", &f, &on_io_error);
    ck_assert(fclose(f) == 0);
    unlink(f_name);
    free(f_name);
}
END_TEST

START_TEST(test_open_file__failure)
{
    jmp_buf on_io_error;
    char *f_name = strdup(MKSTEMP_TEMPLATE);
    int f_fd = mkstemp(f_name);
    FILE *f;

    ck_assert(f_fd >= 0);
    ck_assert(close(f_fd) == 0);
    ck_assert(unlink(f_name) == 0);
    if(!setjmp(on_io_error))
    {
        open_file(f_name, "rb", &f, &on_io_error);
        ck_abort_msg("open_file() was supposed to make a non-local return");
    }
    /* Execution will branch here upon an I/O error */
    free(f_name);
}
END_TEST

START_TEST(test_create_temp_file)
{
    /* Test string must end in a newline */
    static const char DATA[] = "Hello World!\n";
    jmp_buf on_io_error;
    char *orig_file_name = strdup(MKSTEMP_TEMPLATE);
    int orig_fd = mkstemp(orig_file_name);
    FILE *temp_file;
    char temp_file_name[PATH_MAX];
    size_t line_size = strlen(DATA) + 1;
    char *line = malloc(line_size);

    ck_assert(orig_fd >= 0);
    ck_assert(close(orig_fd) == 0);
    if(setjmp(on_io_error))
    {
        /* Execution will branch here upon an I/O error */
        ck_abort_msg("I/O error occurred: %s", strerror(errno));
    }
    create_temp_file(orig_file_name, "wb+", temp_file_name, &temp_file, &on_io_error);
    ck_assert(fputs(DATA, temp_file) >= 0);
    rewind(temp_file);
    ck_assert(fgets(line, line_size, temp_file) != NULL);
    ck_assert(strcmp(line, DATA) == 0);
    ck_assert(fclose(temp_file) == 0);
    ck_assert(unlink(temp_file_name) == 0);
    ck_assert(unlink(orig_file_name) == 0);
    free(line);
    free(orig_file_name);
}
END_TEST

START_TEST(test_replace_file)
{
    /* Test strings must be same length and must end in a newline */
    static const char DATA1[] = "Text1\n";
    static const char DATA2[] = "Text2\n";
    jmp_buf on_io_error;
    char *file1_name = strdup(MKSTEMP_TEMPLATE);
    char *file2_name = strdup(MKSTEMP_TEMPLATE);
    int file1_fd = mkstemp(file1_name);
    int file2_fd = mkstemp(file2_name);
    size_t line_len = strlen(DATA1);
    size_t line_size = line_len + 1;
    char *line = malloc(line_size);
    FILE *result_file;

    ck_assert(write(file1_fd, DATA1, line_len) == (ssize_t)line_len);
    ck_assert(write(file2_fd, DATA2, line_len) == (ssize_t)line_len);
    ck_assert(close(file1_fd) == 0);
    ck_assert(close(file2_fd) == 0);

    if(setjmp(on_io_error))
    {
        /* Execution will branch here upon an I/O error */
        ck_abort_msg("I/O error occurred: %s", strerror(errno));
    }

    replace_file(file1_name, file2_name, &on_io_error);
    result_file = fopen(file1_name, "r");
    ck_assert(result_file != NULL);
    ck_assert(fgets(line, line_size, result_file) != NULL);
    ck_assert(fclose(result_file) == 0);
    ck_assert(unlink(file1_name) == 0);
    free(file1_name);
    free(file2_name);
    free(line);
}
END_TEST

Suite *init_suite(void)
{
    Suite *s = suite_create("filemgmt");
    TCase *tc_core = tcase_create("core");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_open_file__success);
    tcase_add_test(tc_core, test_open_file__failure);
    tcase_add_test(tc_core, test_create_temp_file);
    tcase_add_test(tc_core, test_replace_file);
    suite_add_tcase(s, tc_core);
    return s;
}
