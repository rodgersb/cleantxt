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

/** @file options.h
    Command-line options parsing routines for @c cleantxt. */

#ifndef OPTIONS_H
#define OPTIONS_H

/** The default whitespace mode to use */
#define DEFAULT_WHITESPACE_MODE WM_SPACE

/** The default tab size to use */
#define DEFAULT_TAB_SIZE 8

/** The default minimum whitespace gap length for inserting tabs */
#define DEFAULT_TAB_MIN 2

/** @def DEFAULT_EOL_MODE
    The default end-of-line sequence to use. Should be one of the values
    in #eol_mode_t. */
#ifndef DEFAULT_EOL_MODE
#define DEFAULT_EOL_MODE EM_LF
#endif /* !DEFAULT_EOL_MODE */

/** Enumerations for white space mode */
typedef enum
{
    /** All whitespace will be filled with space characters */
    WM_SPACE,
    /** Tab characters are used to fill whitespace wherever possible.
        Where tabs are not possible, spaces are used to fill remaining
        gaps. If a single-character whitespace gap is encountered, then
        a space character is used to fill it. */
    WM_TAB
} whitespace_mode_t;

/** Enumerations for end-of-line character sequence */
typedef enum
{
    EM_LF = 0,  /**< Use UNIX-style LF for end-of-line sequence */
    EM_CR,      /**< Use MacOS-style CR for end-of-line sequence */
    EM_CRLF     /**< Use DOS-style CR+LF for end-of-line sequence */
} eol_mode_t;

/** Program operation modes as determined by command line arguments */
typedef enum
{
    /** Arguments are yet to be parsed (initial setting) */
    PM_UNKNOWN = 0,
    /** User wanted the help message */
    PM_SHOW_HELP,
    /** User wanted to see the version number */
    PM_SHOW_VERSION,
    /** User wants to process a single file, with differing input/output
        file names. Input file name will be in @a
        options.input_file_name. Output file name will be in @a
        options.output_file_name. If either of these are @c NULL, then
        they correspond to standard input/output respectively. */
    PM_PROCESS_STREAM,
    /** User wants to process one or more files in-place, given in @a
        options.file_name_list. */
    PM_PROCESS_FILE_LIST
} program_mode_t;

/** Command-line argument structure */
struct cleantxt_options
{
    /** Mode of operation that this invocation of the program will follow */
    program_mode_t program_mode;
    /** The size of the tab margins */
    int tab_size;
    /** The minimum length whitespace gaps for filling with tabs */
    int tab_min;
    /** The whitespace fill mode */
    whitespace_mode_t whitespace_mode;
    /** The character sequence to use for end-of-line */
    eol_mode_t eol_mode;
    /** If this flag is set, then a ctrl-Z character signifies the end
        of the input file. */
    unsigned int stop_at_ctrl_z:1;
    /** If this flag is set, then a ctrl-Z character should be appended
        to the end of the file. */
    unsigned int add_ctrl_z:1;
    /** If this flag is set, then any ctrl-Z characters encountered will
        be silently discarded from the input. */
    unsigned int remove_ctrl_z:1;
    /** Points to the input file name. The value of this is only
        meaningful if @c file_name_list is @c NULL. If set to @c NULL,
        then the input file has not been supplied. */
    const char *input_file_name;
    /** Points to the output file name. The value of this is only
        meaningful if @c file_name_list is @c NULL. If set to @c NULL,
        then the output file has not been supplied. */
    const char *output_file_name;
    /** Points to an array of file names to process. The array must be
        terminated with a @c NULL element. file_name_list_length
        elements long. If set to @c NULL, then @c input_file_name is
        used for the input file name and @c output_file_name is used for
        the output file name. */
    const char *const *file_name_list;
};

/** File name used to represent standard input */
extern const char STDIN_FILE_NAME[];

/** File name used to represent standard output */
extern const char STDOUT_FILE_NAME[];

/** Stores @c cleantxt options, as parsed from the command line. */
extern struct cleantxt_options options;

/** Initialises #options structure to sane defaults. Must be called
    before #parse_options(). */
extern void init_options(void);

/** Parses command line arguments and stores them in the #options
    structure. Please call #init_options() before invoking this
    function.

    @param argc Number of command-line arguments, including program name.
    @param argv String array of command-line arguments, including
    program name.
    @param jmp_if_error If a parsing error is encountered, then a
    non-local exit will be made to the address recorded by @c setjmp()
    here.
    @return One of the #program_mode_t constants indicating the outcome
    of the options parsing. If this function returns, then the arguments
    are valid. */
extern void parse_options(int argc, char *const *argv, jmp_buf *jmp_if_error);

/** Prints the program help message to standard output */
extern void print_help_message(void);

/** Prints a message to standard error inviting the user to re-run the
    program with the @c -h option to view the help message. It's helpful
    when they get a value for an argument wrong. */
extern void print_try_help_message(void);

#endif /* !OPTIONS_H */
