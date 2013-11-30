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

/** @file filemgmt.h
    Common miscellaneous routines for disk file management. */

#ifndef FILEMGMT_H
#define FILEMGMT_H

/** Opens a file. If opening the file fails, then an error message will
    be displayed and a non-local exit will be made to the address
    configured by @a jmp_if_error.

    @param file_name Name of the file to open
    @param file_mode The open mode of the file, to be passed to @c
    fopen().
    @param file On return, the pointer variable pointed to by @a file
    will contain the pointer to the stream object associated with the
    file.
    @param jmp_if_error Exception handling address. */
extern void open_file(
    const char *file_name,
    const char *file_mode,
    FILE **file,
    jmp_buf *jmp_if_error);

/** Creates a temporary file in the same directory that @a
    input_file_name resides in. If file creation fails, then an error
    message will be displayed and a non-local exit will be made to the address
    configured by @a jmp_if_error.

    @param file_name The file in which to use as a basis for creating
    the temporary file name.
    @param temp_file_mode This is the open mode of the temporary file,
    passed to @c fopen(). Must start with @c "w".
    @param temp_file_name On return, the temporary file name will be
    stored in the character buffer pointed to by @a temp_file_name. The
    character buffer must be at least @c PATH_MAX characters long.
    @param temp_file On return, the pointer to the stream object will be
    stored here.
    @param jmp_if_error Exception handling address. */
extern void create_temp_file(
    const char *input_file_name,
    const char *temp_file_mode,
    char *temp_file_name,
    FILE **temp_file,
    jmp_buf *jmp_if_error);

/** Closes the given file, if it is not standard input or standard
    output. If an error occurs, then an error message will be displayed
    and a non-local exit will be made to the address configured by @a
    jmp_if_error.

    @param file The stream object to close. If the stream object is @c
    stdin or @c stdout, then no action will be performed. The stream
    object is presumed to be currently open.
    @param file_name The name of the file to close. This is used for
    error message display purposes only.
    @param jmp_if_error Exception handling address. */
extern void close_file(
    FILE *file,
    const char *file_name,
    jmp_buf *jmp_if_error);

/** Deletes the given file. If any errors occur, then an error message
    will be displayed and a non-local exit will be made to the address
    configured by @a jmp_if_error.

    @param file_name The name of the file to delete.
    @param jmp_if_error Exception handling address. */
extern void remove_file(
    const char *file_name,
    jmp_buf *jmp_if_error);

/** Closes and removes the given file, if it is not standard input or
    standard output. If any errors occur, then an error message
    will be displayed and a non-local exit will be made to the address
    configured by @a jmp_if_error.

    @param file The stream object of the file to close and delete. If
    the stream object is @c stdin or @c stdout, then no action will be
    taken. The stream object is presumed to be currently open.
    @param file_name The name of the file to close and delete.
    @param jmp_if_error Exception handling address. */
extern void close_remove_file(
    FILE *file,
    const char *file_name,
    jmp_buf *jmp_if_error);

/** Closes the given file and guarantee that its contents are complete,
    if it is not standard input or standard output. If any errors occur,
    then an error message will be displayed, the file will be removed
    from disk and a non-local exit will be made to the address
    configured by @a jmp_if_error.

    @param file The stream object of the file to close. If the stream
    object is @c stdin or @c stdout, then no action will be taken. The
    stream object is presumed to be currently open.
    @param file_name The name of the file to close. If closing the file
    fails, then this file will be deleted.
    @param jmp_if_error Exception handling address. */
extern void close_file_guarantee_complete_or_remove(
    FILE *file,
    const char *file_name,
    jmp_buf *jmp_if_error);

/** Replaces the given target file with the source file. This is
    performed by deleting the source file, then renaming the target file
    to the source file. The source file is also given the permissions
    and ownership of the target file wherever possible.

    If any errors occur, then an error message will be displayed and a
    non-local exit will be made to the address configured by @a
    jmp_if_error.

    @param target_file_name The name of the file to be replaced. This
    file is deleted and the source file is renamed to this file name.
    @param source_file_name The name of the file to replace the target
    file. This file will be renamed to the source file name.
    @param jmp_if_error Exception handling address. */
extern void replace_file(
    const char *target_file_name,
    const char *source_file_name,
    jmp_buf *jmp_if_error);

#endif /* !FILEMGMT_H */
