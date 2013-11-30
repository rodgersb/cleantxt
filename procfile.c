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

/** @file procfile.c
    Handles processing of files, both one-off and batch list. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <setjmp.h>

#ifdef HAVE_ERROR_H
#   include <error.h>
#else
#   include <compat/error.h>
#endif

#include "procfile.h"
#include "cleanstr.h"
#include "filemgmt.h"
#include "options.h"

/** Definition for boolean constant @e false */
#define FALSE 0

/** Definition for boolean constant @e true */
#define TRUE (!FALSE)

/** @c fopen() mode to use on input streams */
static const char INPUT_MODE[] = "rb";

/** @c fopen() mode to use on output streams */
static const char OUTPUT_MODE[] = "wb";

/** String used to describe standard input to user */
static const char STDIN_DESCRIPTION[] = "<standard input>";

/** String used to describe standard output to user */
static const char STDOUT_DESCRIPTION[] = "<standard output>";

/** Filters a file in-place. This is performed by creating a temporary
    file, writing the output to the temporary file, then deleting the
    original file and renaming the temporary file to the original file.

    To save unnecessary updation of time stamps, if no changes were made
    to the original file, then the temporary file is deleted and the
    original file is left as-is. If any errors occur, then an error
    message will be displayed and the program will be terminated.

    @param input_file_name Name of the input file. Must not be @c NULL.
    Note that @c "-" is <b>NOT</b> recognised as standard input/output. */
static void process_file_in_place(
    const char *input_file_name,
    jmp_buf *jmp_if_error)
{
    /* temp_file_name: Name of the temporary file */
    /* input_file: Input file object stream */
    /* output_file: Temporary output file object stream */
    /* on_clean_stream_error: Handler for I/O errors in clean_stream() */
    char temp_file_name[PATH_MAX];
    FILE *input_file;
    FILE *temp_file;
    jmp_buf on_clean_stream_error;

    /* Open the input file and create a temporary output file. */
    open_file(input_file_name, INPUT_MODE, &input_file, jmp_if_error);
    create_temp_file(input_file_name, OUTPUT_MODE,
        temp_file_name, &temp_file, jmp_if_error);

    /* Filter the input file contents into the temporary file. */
    if(setjmp(on_clean_stream_error))
    {
        /* Execution branches here if clean_stream() encounters an I/O error */
        error(0, errno, "%s", input_file_name);
        close_remove_file(temp_file, temp_file_name, jmp_if_error);
        longjmp(*jmp_if_error, TRUE);
        /* Non-local return */
    }
    switch(clean_stream(input_file, temp_file, &on_clean_stream_error))
    {
        case CSR_STREAM_UNMODIFIED:
            /* No modifications were made to the stream, so the
               temporary file is identical to the input file. So we can
               avoid unnecessarily updating the time stamps on the input
               file, we'll close and remove the temporary file name
               instead. Close the input file as well. */
            close_remove_file(temp_file, temp_file_name, jmp_if_error);
            close_file(input_file, input_file_name, jmp_if_error);
            break;
        case CSR_STREAM_MODIFIED:
            /* Modifications were made to the stream content. The
               temporary file and input files are not identical. Close
               both files, then delete the input file and rename the
               temporary file to the input file. */
            close_file_guarantee_complete_or_remove(
                temp_file, temp_file_name, jmp_if_error);
            close_file(input_file, input_file_name, jmp_if_error);
            replace_file(input_file_name, temp_file_name, jmp_if_error);
            break;
    }
}

void process_file(
    /* Note: the extra `const' is to suppress GCC's warning that these
       arguments would be clobbered by the use of longjmp(). */
    const char *const input_file_name,
    const char *const output_file_name,
    jmp_buf *jmp_if_error)
{
    /* input_file: Stream object associated with the input file */
    /* output_file: Stream object associated with the output file */
    /* on_clean_stream_error: Handler for I/O errors in clean_stream() */
    FILE *input_file;
    FILE *output_file;
    jmp_buf on_clean_stream_error;

    /* Before we open each of the input and output files, we check if
       their names match the special symbol for standard input/output
       respectively. In those cases, link the stream pointer to the C
       library's stdin/stdout objects, and replace the file name pointer
       with a human readable explanation for error messages. */
    if(strcmp(input_file_name, STDIN_FILE_NAME) == 0)
    {
        input_file = stdin;
        /*input_file_name = STDIN_DESCRIPTION;*/
    }
    else
    {
        open_file(input_file_name, INPUT_MODE, &input_file, jmp_if_error);
    }

    if(strcmp(output_file_name, STDOUT_FILE_NAME) == 0)
    {
        output_file = stdout;
        /*output_file_name = STDOUT_DESCRIPTION;*/
    }
    else
    {
        open_file(output_file_name, OUTPUT_MODE, &output_file, jmp_if_error);
    }

    /* Filter the input file to the output file. Abort program with an
       error message if any errors occured. */
    if(setjmp(on_clean_stream_error))
    {
        /* Execution branches here if clean_stream() encounters an I/O error */
        if(ferror(input_file))
        {
            error(0, errno, "%s", ((input_file != stdin)
                ? input_file_name
                : STDIN_DESCRIPTION));
        }
        else
        {
            error(0, errno, "%s", ((output_file != stdout)
                ? output_file_name
                : STDOUT_DESCRIPTION));
        }
        close_remove_file(output_file, output_file_name, jmp_if_error);
        longjmp(*jmp_if_error, TRUE);
        /* Non-local return */
    }
    clean_stream(input_file, output_file, &on_clean_stream_error);
    close_file_guarantee_complete_or_remove(
        output_file, output_file_name, jmp_if_error);
    close_file(input_file, input_file_name, jmp_if_error);
}

void process_file_list(
    const char *const *file_name_index,
    jmp_buf *jmp_if_error)
{
    while(*file_name_index)
    {
        /* Check if the current file name is "-" */
        if(strcmp(*file_name_index, STDIN_FILE_NAME) == 0)
        {
            /* The user specified standard input as a file to replace.
               Filter standard input to standard output. */
            process_file(STDIN_FILE_NAME, STDOUT_FILE_NAME, jmp_if_error);
        }
        else
        {
            /* Process current file in-place */
            process_file_in_place(*file_name_index, jmp_if_error);
        }
        file_name_index++;
    }
}
