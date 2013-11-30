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

/** @file tests/helpers/hexcdiff.h
    Produces a hexadecimal-dump context diff report comparing an
    expected data block with the actual data block, marking the
    differences.

    Features "fuzzy" matching, so it can tell when a few bytes have been
    transposed or inserted/deleted in the actual output, compared to the
    expected output, making it easier to identify logic errors in the
    code. */

#ifndef HELPERS_HEXCDIFF_H
#define HELPERS_HEXCDIFF_H

#include <stddef.h>

/** Maximum data size that #hexcdiff will accept for both the expected
    and actual data buffers. Any bytes beyond these lengths will be
    ignored. This is to keep the overhead of the context searching down
    to a minimum. */
#define HEXCDIFF_MAX_DATA_LEN 4096

/** Computes the in-memory buffer size needed to produce a hex context
    diff given the lengths of the expected and actual data buffers.

    @param expect_buf_len Length of expected data, in bytes.
    @param actual_buf_len Length of actual data, in bytes.
    @return Number of bytes that should be allocated to contain the
    hexadecimal context diff, including the null terminator at the end. */
extern size_t hexcdiff_msg_size(
    size_t expect_buf_len,
    size_t actual_buf_len);

/** Produces a context hexadecimal dump string in memory based on a
    comparison of an expected (what the test case should produce) and
    actual (what the test case did produce) data buffer.

    @param expect_buf Pointer to the start of the expected data buffer.
    @param expect_buf_len Length of the expected data buffer in bytes;
    if it exceeds #HEXCDIFF_MAX_DATA_LEN then any extra data will be
    ignored.
    @param actual_buf Pointer to the start of the actual data buffer
    @param actual_buf_len Length of the actual data buffer in
    bytes;  if it exceeds #HEXCDIFF_MAX_DATA_LEN then any extra data
    will be ignored.
    @param msg Points to a string buffer where the output will be
    stored. This should be a character buffer allocated at least the
    size reported by #hexcdiff_msg_size for the given arguments,
    otherwise a segmentation fault may occur. */
extern void hexcdiff(
    void *expect_buf,
    size_t expect_buf_len,
    void *actual_buf,
    size_t actual_buf_len,
    char *msg);

/** This is an abbreviated interface form of #hexcdiff intended for
    testing purposes. It accepts the expected/actual data buffers as
    null-terimated strings instead, so that while the length is
    auto-computed, the data buffers cannot contain zero-valued bytes.

    @param expect_str Expected data buffer.
    @param actual_str Actual data buffer.
    @return A dynamically allocated string containing the hexadecimal
    dump output. The caller takes the responsibility to deallocate this
    string with @c free() to release the memory. */
extern char *hexcdiff_str_auto(const char *expect_str, const char *actual_str);

#endif /* HELPERS_HEXCDIFF_H */
