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

/** @file tests/helpers/hexdiff.c
    Produces a hexadecimal-dump context diff report comparing an
    expected data block with the actual data block, marking the
    differences. Features "fuzzy" matching, so it can tell when a few
    bytes have been transposed or inserted/deleted in the actual */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "hexcdiff.h"

/** Definition for boolean constant @e false */
#define FALSE 0

/** Definition for boolean constant @e true */
#define TRUE (!FALSE)

#ifndef __GNUC__
#   define __attribute__(x) /* Intentionally left blank */
#endif

/** @c printf() format template for expected rows hex dump margin;
    should take only one integer argument. Must produce a string that is
    exactly #LMARGIN_WIDTH characters long. */
static const char EXP_LMARGIN_FMT[] =  "E:%04x ";

/** @c printf() format template for actual rows hex dump margin; should
    take only one integer argument. Must produce a string that is
    exactly #LMARGIN_WIDTH characters long. */
static const char ACT_LMARGIN_FMT[] = "A:%04x ";

/** Vertical bar separating string for the hex dump. Must be exactly
    #COLS_PER_VBAR characters long. */
static const char VBAR_STR[] = "| ";

/** Format template for each byte in the hex dump. Must take two
    arguments; first is the byte value itself, second is a single
    character that can either be a space or an asterisk (to indicate
    differing bytes). Must produce a string that is exactly
    #COLS_PER_BYTE characters long. */
static const char BYTE_FMT[] = "%02x%c";

/** String used for absent bytes in the hex dump (when one data block is
    larger than the other). Must be #COLS_PER_BYTE characters long
    exactly. */
static const char ABSENT_BYTE_FMT[] = " .%c";

/** Garnish used for separating right-margin ASCII guide from bytes in
    hex dump. Must be exactly #ASCII_LPAD_WIDTH characters long. */
static const char ASCII_LPAD_STR[] = "  ";

/** End-of-line sequence for printing hex dump (may include garnish).
    Should be the length of #ASCII_RPAD_LENGTH plus one. */
static const char EOL_STR[] = "\n";

/** Number of bytes per line in the hex dump. Each byte needs 3 columns.
    The ASCII representation on the right needs one column for each byte
    plus an extra space. */
#define BYTES_PER_LINE 16

/** Bytes per vertical bar symbol in the hex dump (improves
    readability). Should evenly divide #BYTES_PER_LINE. */
#define BYTES_PER_VBAR 4

/** Number of columns per byte in the hex dump */
#define COLS_PER_BYTE 3

/** Number of columns per vertical bar in the hex dump */
#define COLS_PER_VBAR 2

/** Number of vertical bars to be printed per line. */
#define VBARS_PER_LINE ((int)(BYTES_PER_LINE / BYTES_PER_VBAR) - 1)

/** Number of columns occupied by left margin of hex dump (containing
    expected/actual indicator, offset and spacing). */
#define LMARGIN_WIDTH 7

/** Number of extra lines that the hex dump will continue printing if
    the actual data stream is larger than the expected stream. This
    allows the end-user to see the spurious bytes immediately following
    where the end should have been, but avoids the case of faulty code
    getting stuck in an infinite loop and producing an overwhelming
    amount of output that dwarfs the expected version. */
#define NUM_OVERFLOW_LINES 4

/** Marker character to use for bytes in the hex that are correct */
#define CORRECT_BYTE_MARK ' '

/** Marker character to use for bytes in the hex that are replaced
    (character is different, but the following ones are correct).  */
#define REPLACED_BYTE_MARK '*'

/** Marker character to use for bytes appear in place of missing ones in the actual
    output. The following characters match a near-future string in
    the expected output. */
#define MISSING_BYTE_MARK '!'

/** Marker character to use for bytes that have been inserted into the
    actual output. The following characters in the actual output match a
    future string in the expected output. */
#define INSERTED_BYTE_MARK '^'

/** Substitute character to use in the right-margin ASCII guide for
    non-printing characters. */
#define NONPRINT_CHAR '.'

/** Substitute character to use in the right-margin ASCII guide for
    absent characters. */
#define ABSENT_CHAR ' '

/** Number of spacing characters separating hex bytes from ASCII guide
    on right margin. */
#define ASCII_LPAD_WIDTH 2

/** Number of spacing characters padding off the right side of the ASCII
    guide, just before the end of the line. */
#define ASCII_RPAD_WIDTH 0

/** Absolute maximum number of lines being printed - avoids having to
    allocate too much memory. Test cases should be designed to avoid
    exceeding this limit. */
#define ABS_MAX_LINES (HEXCDIFF_MAX_DATA_LEN / BYTES_PER_LINE)

/** Number of bytes needed to print a single row in the hex dump output. */
#define CHARS_PER_ROW \
        (LMARGIN_WIDTH                          /* Margin at left */ \
        + BYTES_PER_LINE * COLS_PER_BYTE        /* Each byte */ \
        + VBARS_PER_LINE * COLS_PER_VBAR        /* V-bar separators */ \
        + ASCII_LPAD_WIDTH + ASCII_RPAD_WIDTH   /* ASCII guide padding */ \
        + BYTES_PER_LINE                        /* ASCII guide at right */ \
        + 1)                                    /* Newline character */

/** If necessary, restricts the length of the two buffer arguments so
    that they do not exceed #HEXCDIFF_MAX_DATA_LEN. Must be called on
    fresh input from outside this module.

    @param expect_buf_len The expected buffer length in bytes, passed by
    reference; if it is greater than #HEXCDIFF_MAX_DATA_LEN, then it
    will be set to #HEXCDIFF_MAX_DATA_LEN upon return.
    @param actual_buf_len The actual buffer length in bytes, passed by
    reference; if it is greater than #HEXCDIFF_MAX_DATA_LEN, then it
    will be set to #HEXCDIFF_MAX_DATA_LEN upon return. */
static void clamp_buf_len(size_t *expect_buf_len, size_t *actual_buf_len)
{
    if(*expect_buf_len > HEXCDIFF_MAX_DATA_LEN)
    {
        *expect_buf_len = HEXCDIFF_MAX_DATA_LEN;
    }
    if(*actual_buf_len > HEXCDIFF_MAX_DATA_LEN)
    {
        *actual_buf_len = HEXCDIFF_MAX_DATA_LEN;
    }
}

/** Given the length of the expected data buffer, computes the maximum
    number of rows that should be printed in the hex dump. Used as a
    guide for memory allocation.

    @param expect_buf_len The expected buffer length in bytes, passed by
    reference. Should be clamped via #clamp_buf_len beforehand.
    @param actual_buf_len The actual buffer length in bytes, passed by
    reference. Should be clamped via #clamp_buf_len beforehand.
    @return The number of lines (expected+actual pairs of rows)
    that will be generated for the hex dump, given the buffer sizes. */
static int max_lines(size_t expect_buf_len, size_t actual_buf_len)
{
    /* expect_lines: Number of rows in expected data hex dump */
    /* actual_lines: Number of rows in actual data hex dump */
    /* max_lines: Maximum number of rows that may need to be printed */
    int expect_lines;
    int actual_lines;
    int max_lines;

    expect_lines = (expect_buf_len + BYTES_PER_LINE - 1) / BYTES_PER_LINE;
    actual_lines = (actual_buf_len + BYTES_PER_LINE - 1) / BYTES_PER_LINE;

    if(actual_lines > expect_lines + NUM_OVERFLOW_LINES)
    {
        max_lines = expect_lines + NUM_OVERFLOW_LINES;
    }
    else
    {
        max_lines = actual_lines;
    }
    return (max_lines < ABS_MAX_LINES)
        ? max_lines
        : ABS_MAX_LINES;
}

/** To confirm missing characters, look for a sequence of this many
    characters that appears immediately in the actual buffer, but a bit
    later in the expected data buffer. */
#define MISSING_SEQ_LEN 8

/** Maximum lookahead offset in the expected buffer for identifying
    missing sequences. */
#define MISSING_SEQ_MAX_LOOKAHEAD 16

/** Performs a look-ahead test to determine if there is a missing
    sequence of characters in the actual data stream.

    This is done by looking for a sequence of #MISSING_SEQ_LEN
    characters that appears at the actual data position, but appears
    later on from the current expected data position.

    @param expect_buf_idx Current index in the expected data buffer
    @param expect_buf_end Points to the first byte beyond the end of the expected
        buffer; used for comparison only, must not be dereferenced.
    @param actual_buf_idx Current index in the actual data buffer
    @param actual_buf_end Points to the first byte beyond the end of the actual
        buffer; used for comparison only, must not be dereferenced.
    @param missing_len Upon return, if a missing sequence is identified,
    then the number of missing characters identified will be stored
    here, otherwise the value pointed to will be left unmodified.
    @return @c TRUE if a sequence of missing characters have been
    identified; @c FALSE otherwise. */
static int is_missing_seq(
    char *expect_buf_idx __attribute__((unused)),
    char *expect_buf_end __attribute__((unused)),
    char *actual_buf_idx __attribute__((unused)),
    char *actual_buf_end __attribute__((unused)),
    size_t *missing_len __attribute__((unused)))
{
    /* FINISH-ME: To be implemented later */
    return FALSE;
}

/** To confirm excess characters, look for a sequence of this many
    characters that appears immediately in the expected buffer, but a bit
    later in the actual data buffer. */
#define EXCESS_SEQ_LEN 8

/** Maximum lookahead offset in the expected buffer for identifying
    excess sequences. */
#define EXCESS_SEQ_MAX_LOOKAHEAD 16

/** Performs a look-ahead test to determine if there is an excess
    sequence of characters in the actual data stream.

    This is done by looking for a sequence of #EXCESS_SEQ_LEN
    characters that appears at the expected data position, but appears
    later on from the current actual data position.

    @param expect_buf_idx Current index in the expected data buffer
    @param expect_buf_end Points to the first byte beyond the end of the expected
        buffer; used for comparison only, must not be dereferenced.
    @param actual_buf_idx Current index in the actual data buffer
    @param actual_buf_end Points to the first byte beyond the end of the actual
        buffer; used for comparison only, must not be dereferenced.
    @param missing_len Upon return, if an excess sequence is identified,
    the number of excess characters identified will be stored here,
    otherwise the value pointed to will be left unmodified.
    @return @c TRUE if a sequence of missing characters have been
    identified; @c FALSE otherwise. */
static int is_excess_seq(
    char *expect_buf_idx __attribute__((unused)),
    char *expect_buf_end __attribute__((unused)),
    char *actual_buf_idx __attribute__((unused)),
    char *actual_buf_end __attribute__((unused)),
    size_t *missing_len __attribute__((unused)))
{
    /* FINISH-ME: To be implemented later */
    return FALSE;
}

/** Produces a mark (or annotation) character for each byte in the
    actual data stream, depending on how it relates to its counterpart
    character in the expected data stream. For example, marking
    characters as replaced, missing or excess.

    @param expect_buf Start of expected data stream
    @param expect_buf_len Length of expected data stream; must not
    exceed #HEXCDIFF_MAX_DATA_LEN.
    @param actual_buf Start of actual data stream
    @param actual_buf_len Length of actual data stream; must not
    exceed #HEXCDIFF_MAX_DATA_LEN.
    @param mark_buf Points to a buffer that will contain an annotation
    mark for each byte. This buffer should be sized @c
    #max_lines*#BYTES_PER_LINE bytes. Caller is responsible for ensuring
    this is the case.
    @param mark_buf_len Length of the mark buffer; this should be
    <code>max_lines(expect_buf_len, actual_buf_len) *
    BYTES_PER_LINE</code>, and is passed simply to avoid having to
    recompute it. */
static void analyse(
    char *expect_buf,
    size_t expect_buf_len,
    char *actual_buf,
    size_t actual_buf_len,
    char *mark_buf,
    size_t mark_buf_len)
{
    /* mark_buf_idx: Current index in mark buffer being written to */
    /* mark_buf_end: Points to the first byte beyond the end of the mark
        buffer; used for comparison only, must not be dereferenced. */
    /* expect_buf_idx: Current comparison index in expected data buffer */
    /* expect_buf_end: Points to the first byte beyond the end of the expected
        buffer; used for comparison only, must not be dereferenced. */
    /* actual_buf_idx: Current comparison index in actual data buffer */
    /* actual_buf_end: Points to the first byte beyond the end of the actual
        buffer; used for comparison only, must not be dereferenced. */
    char *mark_buf_idx = mark_buf;
    char *mark_buf_end = mark_buf + mark_buf_len;
    char *expect_buf_idx = expect_buf;
    char *expect_buf_end = expect_buf + expect_buf_len;
    char *actual_buf_idx = actual_buf;
    char *actual_buf_end = actual_buf + actual_buf_len;

    while(mark_buf_idx < mark_buf_end)
    {
        /* runlen: Used to capture length of missing or excess bytes */
        size_t runlen;

        if(expect_buf_idx >= expect_buf_end
            && actual_buf_idx >= actual_buf_end)
        {
            /* Finishing out the final row */
            *mark_buf_idx++ = CORRECT_BYTE_MARK;
        }
        else if(expect_buf_idx >= expect_buf_end
            && actual_buf_idx < actual_buf_end)
        {
            /* Superflous data on the end of the actual buffer */
            *mark_buf_idx++ = INSERTED_BYTE_MARK;
            actual_buf_idx++;
        }
        else if(expect_buf_idx < expect_buf_end
            && actual_buf_idx >= actual_buf_end)
        {
            /* Actual buffer has fallen short of the expected buffer */
            *mark_buf_idx++ = MISSING_BYTE_MARK;
            expect_buf_idx++;
        }
        else if(expect_buf_idx < expect_buf_end
            && actual_buf_idx < actual_buf_end
            && *expect_buf_idx == *actual_buf_idx)
        {
            /* Correctly matching bytes */
            *mark_buf_idx++ = CORRECT_BYTE_MARK;
            expect_buf_idx++;
            actual_buf_idx++;
        }
        else if(is_missing_seq(
            expect_buf_idx, expect_buf_end,
            actual_buf_idx, actual_buf_end,
            &runlen))
        {
            /* Identified a sequence of characters present in the
               expected buffer, but missing in the actual one. */
            *mark_buf_idx++ = MISSING_BYTE_MARK;
            expect_buf_idx += runlen + 1;
            actual_buf_idx++;
        }
        else if(is_excess_seq(
            expect_buf_idx, expect_buf_end,
            actual_buf_idx, actual_buf_end,
            &runlen))
        {
            /* Identified a sequence of characters present in the actual
               buffer, but not featured in the expected buffer. */
            *mark_buf_idx++ = INSERTED_BYTE_MARK;
            actual_buf_idx += runlen;
        }
        else
        {
            /* Actual and expected bytes mismatch */
            *mark_buf_idx++ = REPLACED_BYTE_MARK;
            expect_buf_idx++;
            actual_buf_idx++;
        }
    }
}

/** Print the hexadecimal rendition for a row of bytes.

    @param msg_ptr Refers to a pointer pointing to the current output
    position in the message buffer. On return this pointer will be
    updated. It is the responsibility of the caller to assume there is
    enough space in the message buffer.
    @param data Points to the bytes to render for this row.
    @param data_len Number of bytes in this row, may be less than
    #BYTES_PER_LINE if at the end of the stream.
    @param data_marks Points to an array of @a data_len characters to
    annotate each hex byte printed. If @c NULL is given, then
    #CORRECT_BYTE_MARK will be printed next to each hex byte instead. */
static void render_bytes_hex(
    char **msg_ptr,
    char *data,
    size_t data_len,
    char *data_marks)
{
    /* i: Byte index for this row */
    /* vbar: Vertical bar counter; when zero is reached, print a
           vertical bar separator, unless at the end of the line. */
    unsigned int i;
    unsigned int vbar = BYTES_PER_VBAR;

    for(i = 0; i < BYTES_PER_LINE; i++)
    {
        int mark = data_marks
            ? data_marks[i]
            : CORRECT_BYTE_MARK;

        if(i < data_len)
        {
            *msg_ptr += sprintf(*msg_ptr, BYTE_FMT, data[i], mark);
        }
        else
        {
            *msg_ptr += sprintf(*msg_ptr, ABSENT_BYTE_FMT, mark);
        }
        vbar--;
        if(vbar == 0 && i < BYTES_PER_LINE - 1)
        {
            *msg_ptr += sprintf(*msg_ptr, "%s", VBAR_STR);
            vbar = BYTES_PER_VBAR;
        }
    }
}

/** Renders the ASCII guide portion of the hex dump for a particular row.

    @param msg_ptr Refers to a pointer pointing to the current output
    position in the message buffer. On return this pointer will be
    updated. It is the responsibility of the caller to assume there is
    enough space in the message buffer.
    @param data Points to the bytes to render for this row.
    @param data_len Number of bytes in this row, may be less than
    #BYTES_PER_LINE if at the end of the stream. */
static void render_bytes_ascii(
    char **msg_ptr,
    char *data,
    size_t data_len)
{
    /* i: Byte index for this row */
    unsigned int i;

    *msg_ptr += sprintf(*msg_ptr, "%s", ASCII_LPAD_STR);
    for(i = 0; i < BYTES_PER_LINE; i++)
    {
        /* c: Character rendered for this byte in the row */
        int c = ABSENT_CHAR;

        if(i < data_len)
        {
            c = isprint(data[i]) ? data[i] : NONPRINT_CHAR;
        }
        *msg_ptr += sprintf(*msg_ptr, "%c", c);
    }
}

/** Renders a line (a pair of rows) of the hex diff output.

    @param msg_ptr Refers to a pointer pointing to the current output
    position in the message buffer. On return this pointer will be
    updated. It is the responsibility of the caller to assume there is
    enough space in the message buffer.
    @param expect_line Points to the next (up to) #BYTES_PER_LINE bytes
    of the expected data to display.
    @param expect_line_len The number of expected bytes available for
    the current line (will be less than #BYTES_PER_LINE if at the end of
    the stream, remaining bytes will be printed as absent).
    @param actual_line Points to the next (up to) #BYTES_PER_LINE bytes
    of the actual data to display.
    @param actual_line_len The number of actual bytes available for
    the current line (will be less than #BYTES_PER_LINE if at the end of
    the stream, remaining bytes will be printed as absent).
    @param offset The offset to print in the left margin of the output.
    @param line_marks Points to #BYTES_PER_LINE characters that are used
    to annotate the actual line of the output (indicating mismatch,
    missing or inserted). */
static void render_line(
    char **msg_ptr,
    char *expect_line,
    size_t expect_line_len,
    char *actual_line,
    size_t actual_line_len,
    int offset,
    char *line_marks)
{
    /* Print expected row */
    *msg_ptr += sprintf(*msg_ptr, EXP_LMARGIN_FMT, offset);
    render_bytes_hex(msg_ptr, expect_line, expect_line_len, NULL);
    render_bytes_ascii(msg_ptr, expect_line, expect_line_len);
    *msg_ptr += sprintf(*msg_ptr, "%s", EOL_STR);

    /* Print actual row */
    *msg_ptr += sprintf(*msg_ptr, ACT_LMARGIN_FMT, offset);
    render_bytes_hex(msg_ptr, actual_line, actual_line_len, line_marks);
    render_bytes_ascii(msg_ptr, actual_line, actual_line_len);
    *msg_ptr += sprintf(*msg_ptr, "%s", EOL_STR);
}

size_t hexcdiff_msg_size(size_t expect_buf_len, size_t actual_buf_len)
{
    /* Must clamp inputs first */
    clamp_buf_len(&expect_buf_len, &actual_buf_len);
    /* Count each row twice and then include the null terminator. */
    return max_lines(expect_buf_len, actual_buf_len) * CHARS_PER_ROW * 2 + 1;
}

/** Computes the length of the current hex dump line, given current
    index pointer and end pointer in a data buffer. */
static size_t line_len(char *data_idx, char *data_end)
{
    if(data_idx < data_end)
    {
        size_t len = data_end - data_idx;
        return (len > BYTES_PER_LINE)
            ? BYTES_PER_LINE
            : len;
    }
    return 0;
}

void hexcdiff(
    void *_expect_buf,
    size_t expect_buf_len,
    void *_actual_buf,
    size_t actual_buf_len,
    char *msg)
{
    char *expect_buf = (char *)_expect_buf;
    char *actual_buf = (char *)_actual_buf;
    char *mark_buf;
    size_t mark_buf_len;
    char *mark_buf_end;
    char *msg_ptr = msg;
    char *mark_buf_idx;
    char *expect_buf_idx = expect_buf;
    char *expect_buf_end = expect_buf + expect_buf_len;
    char *actual_buf_idx = actual_buf;
    char *actual_buf_end = actual_buf + actual_buf_len;
    int offset = 0;

    /* Must clamp inputs first. Then generate annotation marks, and produce the hex dump. */
    clamp_buf_len(&expect_buf_len, &actual_buf_len);
    mark_buf_len = max_lines(expect_buf_len, actual_buf_len) * BYTES_PER_LINE;
    mark_buf = malloc(mark_buf_len);
    mark_buf_end = mark_buf + mark_buf_len;
    mark_buf_idx = mark_buf;
    analyse(expect_buf, expect_buf_len,
        actual_buf, actual_buf_len,
        mark_buf, mark_buf_len);

    while(mark_buf_idx < mark_buf_end)
    {
        size_t expect_line_len = line_len(expect_buf_idx, expect_buf_end);
        size_t actual_line_len = line_len(actual_buf_idx, actual_buf_end);

        render_line(&msg_ptr,
            expect_buf_idx, expect_line_len,
            actual_buf_idx, actual_line_len,
            offset, mark_buf_idx);
        mark_buf_idx += BYTES_PER_LINE;
        expect_buf_idx += expect_line_len;
        actual_buf_idx += actual_line_len;
        offset += BYTES_PER_LINE;
    }

    free(mark_buf);
}

char *hexcdiff_str_auto(const char *expect_str, const char *actual_str)
{
    size_t expect_len = strlen(expect_str);
    size_t actual_len = strlen(actual_str);
    size_t msg_len = hexcdiff_msg_size(expect_len, actual_len);
    char *msg = malloc(msg_len);

    hexcdiff((void *)expect_str, expect_len, (void *)actual_str, actual_len, msg);
    return msg;
}
