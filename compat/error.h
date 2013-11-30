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

/** @file error.h
    Contains a substitute implementation of GNU libc's @c error()
    function for other C libraries that lack it. */

#ifndef COMPAT_ERROR_H
#define COMPAT_ERROR_H

/** Replacement for GNU libc's @c error() function, if the host platform
    doesn't provide one. The given message is printed to standard error.
    An optional text description of an error code can also be displayed.
    The program can also optionally be terminated with a failure exit
    code.

    @param status If non-zero, then the process will be  terminated with
    @a status as the exit code.
    @param errnum If non-zero, then an error description describing the
    @c errno code in @a errnum will be appended to the message.
    @param message The format string for the custom message. This is a
    @c printf() style format string. Arguments for the format string
    should follow afterwards. */
void error(int status, int errnum, const char *message, ...);

#endif /* !COMPAT_ERROR_H */
