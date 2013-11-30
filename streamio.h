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

/** @file streamio.h
    Character stream I/O routines. */

#ifndef STREAMIO_H
#define STREAMIO_H

/** Reads a character from the stream @a file via @c fgetc(). If an I/O
    error is encountered while reading from the stream, then a long jump
    will be made via @c longjmp() to the non-local exit location
    specified by @a jmp_if_error.

    @param jmp_if_error A pointer to the @c jmp_buf instance containing the
    non-local exit location to jump to if an I/O error is encountered.
    @param file The file to read the character from
    @return The character value read from the stream, or @c EOF if
    end-of-file was reached on @a file. */
extern int fgetc_jmp(FILE *file, jmp_buf *jmp_if_error);

/** Writes a character to the stream @a file via @c fputc(). If an I/O
    error is encountered while writing to the stream, then a long jump
    will be made via @c longjmp() to the non-local exit location
    specified by @a jmp_if_error.

    @param jmp_if_error A pointer to the @c jmp_buf instance containing the
    non-local exit location to jump to if an I/O error is encountered.
    @param c The value of the character to write
    @param file The file to write the character to
    @return The value of @a c */
extern int fputc_jmp(int c, FILE *file, jmp_buf *jmp_if_error);

/** Writes a string to the stream @a file via @c fputs(). If an I/O
    error is encountered while writing to the stream, then a long jump
    will be made via @c longjmp() to the non-local exit location
    specified by @a jmp_if_error.

    @param jmp_if_error A pointer to the @c jmp_buf instance containing the
    non-local exit location to jump to if an I/O error is encountered.
    @param s The string to write
    @param file The file to write the string to
    @return A non-negative value */
extern int fputs_jmp(const char *s, FILE *file, jmp_buf *jmp_if_error);

#endif /* !STREAMIO_H */
