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

/** @file tests/helpers/io.c
    Helper I/O routines for performing unit tests */

#ifndef HELPERS_IO_H
#define HELPERS_IO_H

#include <stddef.h>

/** Creates a temporary disk file containing prescribed content, and
    prepares a file stream object for reading from the start.

    @param source_str Content that the file will initially contain.
    It will be interpreted as binary data (no character set translation
    will take place), with the exception that a null terminator
    character (ASCII 0) marks the end of the data. The null terminator
    is not considered part of the data and will not be written.
    @return A file stream object configured to read from the start of
    the file. Upon closing the file, the temporary file will be removed
    from disk. If an I/O error occurs the process will abort via @c
    ck_abort_msg() with an explanation of what went wrong. */
extern FILE *create_input_file_from_str(const char *source_str);

/** Creates a temporary disk file containing prescribed content, and
    prepares a file stream object for reading from the start.

    @param source_buf Memory buffer containing content that the
    file will initially contain. It will be interpreted as binary data
    (no character set translation will take place).
    @param source_buf_len Length of data buffer, in bytes.
    @return A file stream object configured to read from the start of
    the file. Upon closing the file, the temporary file will be removed
    from disk. If an I/O error occurs the process will abort via @c
    ck_abort_msg() with an explanation of what went wrong. */
extern FILE *create_input_file_from_buf(
    void *source_buf, size_t source_buf_len);

/** Compares the content in an open file to that of a reference string.

    Any differences in the data will result in the two data streams
    being printed to standard error as a hexadecimal dump (with ASCII
    representation on the side) and the differing bytes flagged with an
    asterisk.

    Note that this function only returns if the reference data and the
    output file match. If the reference data and the output file
    contents don't match, or an I/O error occurs, then the process will
    terminate via @c ck_abort_msg().

    @param expect_str The reference string used for comparison. It will be
    interpreted as binary data (no character set translation will take
    place), with the exception that a null terminator character (ASCII
    0) marks the end of the data. The null terminator is not considered
    part of the data.
    @param actual_file The file to compare contents against. This file
    must be open for reading in binary mode. The file position will be
    left at the end upon return if the comparison completed. */
extern void assert_output_file_contents_match_str(
    const char *expect_str,
    FILE *actual_file);

/** Compares the content in an open file to that of a reference buffer.

    Any differences in the data will result in the two data streams
    being printed to standard error as a hexadecimal dump (with ASCII
    representation on the side) and the differing bytes flagged with an
    asterisk.

    Note that this function only returns if the reference data and the
    output file match. If the reference data and the output file
    contents don't match, or an I/O error occurs, then the process will
    terminate via @c ck_abort_msg().

    @param expect_buf The reference buffer used for comparison. It will be
    interpreted as binary data (no character set translation will take
    place).
    @param expect_buf_len Length of the reference buffer in bytes.
    @param actual_file The file to compare contents against. This file
    must be open for reading in binary mode. The file position will be
    left at the end upon return if the comparison completed */
extern void assert_output_file_contents_match_buf(
    void *expect_buf,
    size_t expect_buf_len,
    FILE *actual_file);

#endif /* HELPERS_IO_H */
