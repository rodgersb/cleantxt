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

/** @file filemgmt.c
    Common miscellaneous routines for disk file management. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <setjmp.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifdef HAVE_ERROR_H
#   include <error.h>
#else
#   include <compat/error.h>
#endif /* HAVE_ERROR_H */

#ifdef HAVE_LIBGEN_H
#include <libgen.h> /* for basename() */
#endif /* HAVE_LIBGEN_H */

#include "filemgmt.h"

/** Definition for boolean constant @e false */
#define FALSE 0

/** Definition for boolean constant @e true */
#define TRUE (!FALSE)

void open_file(
    const char *file_name,
    const char *file_mode,
    FILE **file,
    jmp_buf *jmp_if_error)
{
    *file = fopen(file_name, file_mode);
    if(!*file)
    {
        error(0, errno, "%s", file_name);
        longjmp(*jmp_if_error, TRUE);
    }
}

void create_temp_file(
    const char *input_file_name,
    const char *temp_file_mode,
    char *temp_file_name,
    FILE **temp_file,
    jmp_buf *jmp_if_error)
{
    /* temp_fd: File descriptor of the temporary file */
    /* input_file_base_name: Storage for the base name of
       input_file_name. This is used in order to determine the directory
       name of input_file_name, but without having to know what the host
       environment's directory separator characters are. */
    int temp_fd;
    char input_file_base_name[PATH_MAX];

    /* Create a template name for the temporary file, in an OS-neutral
       manner. This is performed by extracting the base name of
       input_file_name, copying input_file_name to temp_file_name, then
       overwriting the base name portion in temp_file_name with the
       temporary base name. Since the basename() call may modify the
       string you supply it with, we need to make a scratch copy of
       input_file_name first; use temp_file_name to hold the scratch
       copy, to save us from allocating another string buffer. */
    strncpy(temp_file_name, input_file_name, PATH_MAX - 1);
    temp_file_name[PATH_MAX - 1] = 0;
    strncpy(input_file_base_name, basename(temp_file_name), PATH_MAX - 1);
    input_file_base_name[PATH_MAX - 1] = 0;
    strncpy(temp_file_name, input_file_name, PATH_MAX - 1);
    strcpy(temp_file_name + strlen(input_file_name)
        - strlen(input_file_base_name), "XXXXXX");

    /* Create the temporary file */
    temp_fd = mkstemp(temp_file_name);
    if(temp_fd < 0)
    {
        error(0, errno, "unable to create temporary file `%s'", temp_file_name);
        longjmp(*jmp_if_error, TRUE);
    }
    *temp_file = fdopen(temp_fd, temp_file_mode);
}

void close_file(
    FILE *file,
    const char *file_name,
    jmp_buf *jmp_if_error)
{
    if(file != stdin && file != stdout && fclose(file) < 0)
    {
        error(0, errno, "unable to close file `%s'", file_name);
        longjmp(*jmp_if_error, TRUE);
    }
}

void remove_file(const char *file_name, jmp_buf *jmp_if_error)
{
    if(remove(file_name) < 0)
    {
        error(0, errno, "unable to remove file `%s'", file_name);
        longjmp(*jmp_if_error, TRUE);
    }
}

void close_remove_file(FILE *file, const char *file_name, jmp_buf *jmp_if_error)
{
    if(file != stdin && file != stdout)
    {
        close_file(file, file_name, jmp_if_error);
        remove_file(file_name, jmp_if_error);
    }
}

void close_file_guarantee_complete_or_remove(
    FILE *file, const char *file_name, jmp_buf *jmp_if_error)
{
    if(file != stdin && file != stdout)
    {
        jmp_buf on_io_error;

        if(setjmp(on_io_error))
        {
            /* Execution will branch here if closing the file fails. It
               usually means flushing the userspace buffer caused an I/O
               error and the contents are incomplete. Remove the file
               and report an error condition. */
            remove_file(file_name, jmp_if_error);
            longjmp(*jmp_if_error, TRUE);
            /* Non-local return */
        }
        close_file(file, file_name, &on_io_error);
    }
}

void replace_file(
    const char *target_file_name,
    const char *source_file_name,
    jmp_buf *jmp_if_error)
{
    /* target_file_stat: Attributes of target file */
    struct stat target_file_stat;

    /* Set the permissions and owner of the source file to the same as
       the target file. Ignore any errors that may occur. This is
       especially in the case of running as a non-root user, in which
       case changing ownership is not permitted. */
    stat(target_file_name, &target_file_stat);
    chmod(source_file_name, target_file_stat.st_mode);
    chown(source_file_name, target_file_stat.st_uid, target_file_stat.st_gid);

    /* Rename the source file to the target file in an atomic manner
       to avoid security exploits in publicly-writable directories. */
    if(rename(source_file_name, target_file_name) < 0)
    {
        error(0, errno, "%s", target_file_name);
        remove_file(source_file_name, jmp_if_error);
        longjmp(*jmp_if_error, TRUE);
    }
}
