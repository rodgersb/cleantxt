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

/** @file tests/helpers/runner.c
    Contains the @c main() function for each test program. */

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <check.h>

/** Each test suite module needs to export a function named @c
    init_suite(), taking no arguments and returning a pointer to a check
    @c Suite instance. This allows each module to take care of its own
    setup/teardown functions as needed. */
extern Suite *init_suite(void);

/** Entry point for each test program */
int main()
{
    int num_failed;
    SRunner *sr;
    Suite *s = init_suite();

    sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    num_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (num_failed == 0)
        ? EXIT_SUCCESS
        : EXIT_FAILURE;
}
