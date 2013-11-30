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

/** @file tests/helpers/testio.c
    Helper I/O routines for performing unit tests */

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <check.h>

#include "io.h"
#include "hexcdiff.h"

/** Definition for boolean constant @e false */
#define FALSE 0

/** Definition for boolean constant @e true */
#define TRUE (!FALSE)

/** Conditionally aborts the process via @c ck_abort_msg with an error
    message explaining an I/O error disrupted the test.

    @param x A boolean expression that should evaluate to true if an I/O
    error occurred. For most C library calls, this would be either
    testing for a negative return value or a @c NULL pointer. It is
    guaranteed that the expression @a x will only be evaluated once. */
#define ABORT_IO_ERROR_IF(x) \
    ck_assert_msg(!(x), \
        "An I/O error was encountered while reading/writing from/to temporary files: %s", \
        strerror(errno))

FILE *create_input_file_from_str(const char *source_str)
{
    return create_input_file_from_buf((void *)source_str, strlen(source_str));
}

FILE *create_input_file_from_buf(void *source_buf, size_t source_buf_len)
{
    /* f: input file created, to be returned to caller. */
    FILE *f = tmpfile();

    ABORT_IO_ERROR_IF(!f
        || fwrite(source_buf, 1, source_buf_len, f) < source_buf_len
        || fseek(f, 0, SEEK_SET) != 0);
    return f;
}

void assert_output_file_contents_match_str(
    const char *expect_str,
    FILE *actual_file)
{
    assert_output_file_contents_match_buf(
        (void *)expect_str, strlen(expect_str), actual_file);
}

/** Compares an in-memory buffer to an open disk file.

    @param expect_buf Reference buffer in-memory.
    @param expect_buf_len Reference buffer length in bytes.
    @param actual_f File to compare against reference buffer. The file must be
    open for reading. The file position will undefined on return.
    @return @c TRUE if the file contents and the reference data match.
    @c FALSE if either they don't match (@c errno will be zero). If an
    I/O error occurs then the process will terminate via @c
    ck_abort_msg(). */
static int file_contents_match_buf(
    void *expect_buf, size_t expect_buf_len, FILE *actual_file)
{
    /* ref_idx: Index into reference buffer of current byte for comparison */
    /* c: Most recent character read from file */
    const char *expect_buf_idx = (const char *)expect_buf;
    int c;

    rewind(actual_file);
    while(expect_buf_len-- > 0)
    {
        c = getc(actual_file);
        if(c == EOF || c != *expect_buf_idx++)
        {
            ABORT_IO_ERROR_IF(ferror(actual_file));
            /* Unexpected end-of-file or data mismatch discovered. */
            return FALSE;
        }
    }

    c = fgetc(actual_file);
    if(c != EOF)
    {
        /* Excess characters in file */
        return FALSE;
    }
    return TRUE;
}

/** Prologue text for the hex dump */
static const char HEX_DUMP_PROLOGUE_STR[] =
    "Expected and actual output file content mismatch. Hex dump follows:\n";

void assert_output_file_contents_match_buf(
    void *expect_buf,
    size_t expect_buf_len,
    FILE *actual_file)
{
    if(!file_contents_match_buf(expect_buf, expect_buf_len, actual_file))
    {
        /* actual_buf_len: Size of actual content, in bytes. */
        /* actual_file_pos: End position offset in actual file */
        /* actual_buf: Memory-allocated buffer holding actual file content */
        /* msg: Message used for aborting test; needs to be constructed in buffer. */
        /* msg_len: Length of abort message buffer, in bytes. */
        /* msg_idx: Point in message buffer where hex dump will be assembled. */
        size_t actual_buf_len;
        long int actual_file_pos;
        void *actual_buf;
        char *msg;
        size_t msg_len;
        char *msg_idx;

        /* Determine size of actual file, load it into a memory buffer. */
        ABORT_IO_ERROR_IF(fseek(actual_file, 0, SEEK_END));
        actual_file_pos = ftell(actual_file);
        ABORT_IO_ERROR_IF(actual_file_pos < 0);
        actual_buf_len = (size_t)actual_file_pos;
        actual_buf = malloc(actual_buf_len);
        rewind(actual_file);
        ABORT_IO_ERROR_IF(fread(actual_buf, 1, actual_buf_len, actual_file) < actual_buf_len);

        /* Allocate string to build test abort message (containing hex dump). */
        msg_len = strlen(HEX_DUMP_PROLOGUE_STR)
            + hexcdiff_msg_size(expect_buf_len, actual_buf_len);
        msg = malloc(msg_len);
        strcpy(msg, HEX_DUMP_PROLOGUE_STR);
        msg_idx = msg + strlen(HEX_DUMP_PROLOGUE_STR);

        /* Build hex dump, then display it as the test abort message. */
        hexcdiff(expect_buf, expect_buf_len, actual_buf, actual_buf_len, msg_idx);
        ck_abort_msg(msg);
    }
}
