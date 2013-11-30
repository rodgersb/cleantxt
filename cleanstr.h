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

/** @file cleanstr.h
    This is the gist of the text-cleaning algorithm. */

#ifndef CLEANSTR_H
#define CLEANSTR_H

/** Return status codes for #clean_stream */
typedef enum
{
    CSR_STREAM_MODIFIED,    /**< The stream was modified */
    CSR_STREAM_UNMODIFIED   /**< The stream was unchanged */
} clean_stream_result_t;

/** Performs stream filtering. Input is read from @a in_stream and
    processed text is written to @a out_stream. Processing stops when the
    end-of-file is reached on @a in_stream, however both @a in_stream and
    @a out_stream are left open.

    @param in_stream The input stream
    @param out_stream The output stream
    @param jmp_if_error Error handler invoked if an I/O error occurs.
    @returns One of the following is returned:
    @li #CSR_STREAM_MODIFIED if the operation was successful, and
    modifications were made to the stream.
    @li #CSR_STREAM_UNMODIFIED if the operation was successful, but no
    modifications were made to the stream. The output stream contains
    the contents of the input stream. */
extern clean_stream_result_t clean_stream(
    FILE *in_stream,
    FILE *out_stream,
    jmp_buf *jmp_if_error);

#endif /* !CLEANSTR_H */
