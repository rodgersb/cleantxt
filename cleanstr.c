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

/** @file cleanstr.c
    This is the gist of the text-cleaning algorithm. */

#include <stdio.h>
#include <setjmp.h>

#include "cleanstr.h"
#include "options.h"
#include "streamio.h"

/** Definition for boolean constant @e false */
#define FALSE 0

/** Definition for boolean constant @e true */
#define TRUE (!FALSE)

/** Constant for ASCII tab character */
#define CHAR_TAB 9

/** Constant for ASCII LF character */
#define CHAR_LF 10

/** Constant for ASCII CR character */
#define CHAR_CR 13

/** Constant for DOS EOF character */
#define CHAR_EOF 26

/** Constant for ASCII space character */
#define CHAR_SPACE 32

/** Computes the column position of the next tab stop.

    @param column The column position on the line where the search for
    the next tab stop should begin. @c 0 refers to the first column of
    the line. Note that this expression may be evaluated multiple times.
    @param tab_size The size of the tab stops. Note that this expression
    may be evaluated multiple times.
    @return The column position of the next tab stop after @a column. */
#define NEXT_TAB_STOP(column, tab_size) \
    ((column) + (tab_size) - (column) % (tab_size))

/** End-of-line strings for end-of-line modes. Each element in this
    array should correspond to the equivalent indexed value in
    #eol_mode_t. */
static const char *const EOL_STR[] =
{
    "\n",
    "\r",
    "\r\n"
};

/** Flushes the collected sequence of whitespace to the output stream.

    @param out_stream The output stream
    @param in_col The current column position on the current line in the
    input file. Note that columns are zero-based.
    @param out_col The current column position on the current line in
    the output file. Note that columns are zero-based.
    @param collected_spaces The number of collected spaces
    @param collected_tabs The number of collected tabs
    @param collected_newlines The number of collected EOL sequences
    @param new_out_col On return, the current column position of the
    current line in the output file will be stored in the integer
    variable pointed to by @a new_out_col. Note that columns are
    zero-based. If an I/O error occurs, then the value of the integer
    variable pointed to by @a new_out_col will be undefined.
    @param csr If the filtering operation results in the stream content
    being modified, then the #clean_stream_result_t variable pointed to
    by @a csr will be set to #CSR_STREAM_MODIFIED. Otherwise the
    variable will be left as-is.
    @param jmp_if_error If an I/O error occurs, then a long jump will be made to the
    location specified in the @c jmp_buf instance pointed to by @a jmp_if_error. */
static void flush_whitespace(
    FILE *out_stream,
    int in_col,
    int out_col,
    int collected_spaces,
    int collected_tabs,
    int collected_newlines,
    int *new_out_col,
    clean_stream_result_t *csr,
    jmp_buf *jmp_if_error)
{
    /* Flush out any accumulated end-of-line sequences, using the character
       sequence that the user specified on the command line. */
    if(collected_newlines > 0)
    {
        do
        {
            fputs_jmp(EOL_STR[options.eol_mode], out_stream, jmp_if_error);
            collected_newlines--;
        }
        while(collected_newlines > 0);
        out_col = 0;
    }

    /* If tab characters are enabled, we output as many tab characters
       as possible to save disk space. Only expand tabs if the
       whitespace gap is at least the minimum threshold for tab
       insertion. */
    if(options.whitespace_mode == WM_TAB && in_col - out_col >= options.tab_min)
    {
        /* Keep inserting tab characters until the next tab stop is
           beyond the current input stream column position. */
        while(NEXT_TAB_STOP(out_col, options.tab_size) <= in_col)
        {
            /* Flush a collected tab character to the output stream */
            fputc_jmp(CHAR_TAB, out_stream, jmp_if_error);
            out_col = NEXT_TAB_STOP(out_col, options.tab_size);
            collected_tabs--;
        }
    }

    /* Fill up the rest of the gap with space characters */
    while(out_col < in_col)
    {
        fputc_jmp(CHAR_SPACE, out_stream, jmp_if_error);
        out_col++;
        collected_spaces--;
    }

    /* If the whitespace gap was reformatted, then indicate to the
       caller that the stream was modified. */
    if(collected_spaces != 0 || collected_tabs != 0)
    {
        *csr = CSR_STREAM_MODIFIED;
    }

    /* Update the caller's output column position */
    *new_out_col = out_col;
}

/** Collects up a sequence of whitespace from the given input stream.
    The input stream will be left at either the next non-whitespace
    character, or end-of-file. The filtered whitespace sequence will be
    written to the output file if a non-whitespace character was
    encountered on the input file.

    @param in_stream The input stream
    @param out_stream The output stream
    @param in_col The current column position on the current line in the
    input file. Note that columns are zero-based.
    @param new_out_col On return, the current column position of the
    current line in the output file will be stored in the integer
    variable pointed to by @a new_out_col. Note that columns are
    zero-based. If an I/O error occurs, then the value of the integer
    variable pointed to by @a new_out_col will be undefined.
    @param csr If the filtering operation results in the stream content
    being modified, then the #clean_stream_result_t variable pointed to
    by @a csr will be set to #CSR_STREAM_MODIFIED.  Otherwise the
    variable will be left as-is.
    @param jmp_if_error If an I/O error occurs, then a long jump will be made to the
    location specified in the @c jmp_buf instance pointed to by @a jmp_if_error. */
static void collect_whitespace(
    FILE *in_stream,
    FILE *out_stream,
    int in_col,
    int *new_out_col,
    clean_stream_result_t *csr,
    jmp_buf *jmp_if_error)
{
    /* c: Contains the current character read from the input stream */
    /* collected_spaces: The number of spaces that have been read */
    /* collected_tabs: The number of tab characters that have been read */
    /* collected_newlines: The number of end-of-line sequences that have been read */
    /* found_non_whitespace: Set this flag when non-whitespace character reached */
    /* in_col: Save the current output column position in out_col */
    int c = 0;
    int collected_spaces = 0;
    int collected_tabs = 0;
    int collected_newlines = 0;
    int found_non_whitespace = FALSE;
    int out_col = in_col;

    /* Continue reading characters in the input stream until we hit
       a non-whitespace character */
    do
    {
        /* found_eol: Set this flag when we encounter an end-of-line sequence */
        /* eol_type: If found_eol is set, then this contains the type of
           end-of-line sequence found. */
        int found_eol = FALSE;
        eol_mode_t eol_type;

        c = fgetc_jmp(in_stream, jmp_if_error);
        switch(c)
        {
            case CHAR_SPACE:
                /* Collect up individual spaces */
                in_col++;
                collected_spaces++;
                break;
            case CHAR_TAB:
                /* Collect up tabs, compute number of spaces required to fill. */
                in_col = NEXT_TAB_STOP(in_col, options.tab_size);
                collected_tabs++;
                if(collected_spaces > 0)
                {
                    /* If there are any spaces that precede this tab,
                       then either they will get substituted with a tab
                       character, or the tab character will get expanded
                       into spaces. Either way, the stream is being
                       modified, so indicate this to the caller. */
                    *csr = CSR_STREAM_MODIFIED;
                }
                break;
            case CHAR_CR:
                /* Encountered a CR character. Check if the next
                   character in the stream is a LF character. */
                found_eol = TRUE;
                if(ungetc(fgetc_jmp(in_stream, jmp_if_error), in_stream) == CHAR_LF)
                {
                    /* We found a CR+LF end-of-line sequence. Skip the
                       LF character. */
                    eol_type = EM_CRLF;
                    fgetc(in_stream);
                }
                else
                {
                    /* Found a CR-only end-of-line sequence */
                    eol_type = EM_CR;
                }
                break;
            case CHAR_LF:
                /* Found a LF-only end-of-line sequence */
                found_eol = TRUE;
                eol_type = EM_LF;
                break;
            case EOF:
                /* We've either hit end-of-file or an I/O error */
                break;
            default:
                /* Found a non-whitespace character. Stop reading and
                   push the character back on to the input stream. */
                found_non_whitespace = TRUE;
                ungetc(c, in_stream);
                break;
        }

        /* Check if we encountered an EOL sequence */
        if(found_eol)
        {
            if(eol_type != options.eol_mode)
            {
                /* If the EOL sequence encountered is different to the
                   one we are outputting, then it will get transformed;
                   indicate to the caller that the stream will be
                   modified. */
                *csr = CSR_STREAM_MODIFIED;
            }

            if(collected_spaces > 0 || collected_tabs > 0)
            {
                /* If there are any trailing tabs or spaces, they will
                   be deleted. Indicate to the caller that the stream
                   will be modified. */
                collected_spaces = 0;
                collected_tabs = 0;
                *csr = CSR_STREAM_MODIFIED;
            }

            /* Move the input column position back to the start and
               collect the EOL sequence. */
            in_col = 0;
            collected_newlines++;
            found_eol = FALSE;
        }
    }
    while(c != EOF && !found_non_whitespace);

    if(found_non_whitespace)
    {
        /* We've encountered a non-whitespace character. Flush all
           pending whitespace to the output stream. */
        flush_whitespace(out_stream, in_col, out_col,
            collected_spaces, collected_tabs, collected_newlines,
            new_out_col, csr, jmp_if_error);
    }
    /* We've encountered the end of the input stream */
    else if(out_col > 0)
    {
        /* If at least one non-whitespace character exists on the
           current line, then write a final EOL sequence to the output
           stream. This helps eliminate a common problem where shell
           interpreters and compilers (like gcc) complain about a
           missing newline character on the end of the last line of the
           file, or worse yet completely ignore the last line of the
           file. If the input stream is empty, or contains only
           whitespace, then the output stream will remain blank. */
        fputs_jmp(EOL_STR[options.eol_mode], out_stream, jmp_if_error);

        if(collected_newlines != 1
            || collected_spaces != 0
            || collected_tabs != 0)
        {
            /* If there is any whitespace other than a single EOL
               sequence following the last non-whitespace character,
               then this will be filtered out. Let the caller know. */
            *csr = CSR_STREAM_MODIFIED;
        }
    }
}

clean_stream_result_t clean_stream(
    FILE *in_stream,
    FILE *out_stream,
    jmp_buf *jmp_if_error)
{
    /* csr: Value to return to the caller */
    /* col: Current column position in the file */
    /* c: Current non-whitespace character in the input stream */
    /* last_c: Previous non-whitespace character in the input stream */
    clean_stream_result_t csr = CSR_STREAM_UNMODIFIED;
    int col = 0;
    int c = 0;
    int last_c = 0;

    /* Continue filtering characters until either we reach
       end-of-file, or we encounter a significant end-of-file
       marker. If an I/O error is encountered, then a non-local exit
       will be made to the error handler that the caller set up. */
    do
    {
        collect_whitespace(in_stream, out_stream, col, &col, &csr, jmp_if_error);
        last_c = c;
        c = fgetc_jmp(in_stream, jmp_if_error);

        if(c == CHAR_EOF && (options.remove_ctrl_z || options.stop_at_ctrl_z))
        {
            /* Deleted a ctrl-z character. In the case of
               stop_at_ctrl_z, we need to re-add it later. */
            csr = CSR_STREAM_MODIFIED;
            /* If ctrl-z characters mark explicit end-of-file, then marking
               the file as modified (even if no changes were made up to this
               point) causes any content that might be past the ctrl-z
               character to be consistently discarded. This also spares us
               from having to read past the ctrl-z character; in
               extreme circumstances, say, if the input is coming over a
               pipe and the sender intends to re-use the pipe for other
               purposes afterwards, this could cause deadlock. */
        }
        else if(c != EOF)
        {
            fputc_jmp(c, out_stream, jmp_if_error);
            col++;
        }
    }
    while(c != EOF && !(options.stop_at_ctrl_z && c == CHAR_EOF));


    /* End of stream was reached */
    if((options.add_ctrl_z && last_c != CHAR_EOF) || options.stop_at_ctrl_z)
    {
        if(col > 0 && options.stop_at_ctrl_z)
        {
            fputs_jmp(EOL_STR[options.eol_mode], out_stream, jmp_if_error);
        }
        fputc_jmp(CHAR_EOF, out_stream, jmp_if_error);
        csr = CSR_STREAM_MODIFIED;
    }

    return csr;
}
