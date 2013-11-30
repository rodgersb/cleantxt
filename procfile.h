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

/** @file cleantxt.h
    Handles processing of files, both one-off and batch list. */

#ifndef PROCFILE_H
#define PROCFILE_H

/** Reads an input file and writes filtered output to a given output
    file. If any errors are encountered during the operation, then an
    error message will be displayed and the program will terminate with
    failure status.

    @param input_file_name The name of the input file. If @a
    input_file_name is @c "-", then standard input will be used. Must
    not be @c NULL.
    @param output_file_name The name of the output file. If @a
    output_file_name is @c "-", then standard output will be used. Must
    not be @c NULL. */
extern void process_file(
    const char *input_file_name,
    const char *output_file_name,
    jmp_buf *jmp_if_error);

/** Processes a list of files in-place. If an error occurs while
    processing any of the files, then an error message will be displayed
    and the program will terminate with failure status.

    @param file_name_index An array containing the file name of each
    file to filter in-place. The array must be terminated with a @c NULL
    element. If an element contains @c "-", then standard input will be
    filtered to standard output. */
extern void process_file_list(
    const char *const *file_name_index,
    jmp_buf *jmp_if_error);

#endif /* !PROCFILE_H */
