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

/** @file tests/ckstrmio.c
    Test suite for streamio module. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>
#include <check.h>

#include "../streamio.h"
#include "helpers/io.h"

START_TEST(test_fgetc_jmp__success)
{
    static const char DATA[] = "x";
    jmp_buf on_io_error;
    FILE *f = create_input_file_from_str(DATA);

    if(setjmp(on_io_error))
    {
        /* Execution branches here on I/O error */
        ck_abort_msg("I/O error occurred");
    }

    ck_assert(f != NULL);
    ck_assert(fgetc_jmp(f, &on_io_error) == DATA[0]);
    ck_assert(fgetc_jmp(f, &on_io_error) == EOF);
    ck_assert(fclose(f) == 0);
}
END_TEST

START_TEST(test_fgetc_jmp__failure)
{
    jmp_buf on_io_error;

    if(!setjmp(on_io_error))
    {
        fgetc_jmp(stdout, &on_io_error);
        ck_abort_msg("fgetc_jmp() was supposed to exit via longjmp()");
    }
    /* Execution branches here if fgetc_jmp() makes a non-local exit */
}
END_TEST

START_TEST(test__fputc_jmp__success)
{
    static const char DATA[] = "x";
    jmp_buf on_io_error;
    FILE *f = tmpfile();

    if(setjmp(on_io_error))
    {
        /* Execution branches here on I/O error */
        ck_abort_msg("I/O error occurred");
    }

    ck_assert(f != NULL);
    ck_assert(fputc_jmp(DATA[0], f, &on_io_error) == DATA[0]);
    rewind(f);
    ck_assert(fgetc(f) == DATA[0]);
    ck_assert(fgetc(f) == EOF);
    ck_assert(fclose(f) == 0);
}
END_TEST

START_TEST(test__fputc_jmp__failure)
{
    jmp_buf on_io_error;

    if(!setjmp(on_io_error))
    {
        fputc_jmp('x', stdin, &on_io_error);
        ck_abort_msg("fputc_jmp() was supposed to exit via longjmp()");
    }
    /* Execution branches here if fgetc_jmp() makes a non-local exit */
}
END_TEST

START_TEST(test__fputs_jmp__success)
{
    static const char DATA[] = "Hello World!\n";
    jmp_buf on_io_error;
    FILE *f = tmpfile();
    size_t line_size = strlen(DATA) + 1;
    char *line = malloc(line_size);

    if(setjmp(on_io_error))
    {
        /* Execution branches here on I/O error */
        ck_abort_msg("I/O error occurred");
    }
    ck_assert(f != NULL);
    ck_assert(fputs_jmp(DATA, f, &on_io_error) >= 0);
    rewind(f);
    ck_assert(fgets(line, line_size, f) != NULL);
    ck_assert(strcmp(line, DATA) == 0);
    ck_assert(fclose(f) == 0);
}
END_TEST

START_TEST(test__fputs_jmp__failure)
{
    jmp_buf on_io_error;

    if(!setjmp(on_io_error))
    {
        fputs_jmp("x", stdin, &on_io_error);
        ck_abort_msg("fputc_jmp() was supposed to exit via longjmp()");
    }
    /* Execution branches here if fgets_jmp() makes a non-local exit */
}
END_TEST

Suite *init_suite(void)
{
    Suite *s = suite_create("streamio");
    TCase *tc_core = tcase_create("core");
    tcase_add_test(tc_core, test_fgetc_jmp__success);
    tcase_add_test(tc_core, test_fgetc_jmp__failure);
    tcase_add_test(tc_core, test__fputc_jmp__success);
    tcase_add_test(tc_core, test__fputc_jmp__failure);
    tcase_add_test(tc_core, test__fputs_jmp__success);
    tcase_add_test(tc_core, test__fputs_jmp__failure);
    suite_add_tcase(s, tc_core);
    return s;
}
