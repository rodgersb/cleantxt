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

/** @file streamio.c
    Character stream I/O routines. */

#include <stdio.h>
#include <setjmp.h>
#include "streamio.h"

/** Definition for boolean constant @e false */
#define FALSE 0

/** Definition for boolean constant @e true */
#define TRUE (!FALSE)

int fgetc_jmp(FILE *file, jmp_buf *jmp_if_error)
{
    int fgetc_result = fgetc(file);
    if(fgetc_result == EOF && ferror(file))
    {
        longjmp(*jmp_if_error, TRUE);
    }
    return fgetc_result;
}

int fputc_jmp(int c, FILE *file, jmp_buf *jmp_if_error)
{
    int fputc_result = fputc(c, file);
    if(fputc_result == EOF)
    {
        longjmp(*jmp_if_error, TRUE);
    }
    return fputc_result;
}

int fputs_jmp(const char *s, FILE *file, jmp_buf *jmp_if_error)
{
    int fputs_result = fputs(s, file);
    if(fputs_result == EOF)
    {
        longjmp(*jmp_if_error, TRUE);
    }
    return fputs_result;
}
