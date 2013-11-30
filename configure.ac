dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.50)
AC_INIT(cleantxt, 1.0.1, [rodgersb@it.net.au])
AM_INIT_AUTOMAKE(cleantxt, 1.0.1)
AM_CONFIG_HEADER(config.h)

dnl Check if a C compiler is available
AC_PROG_CC

dnl Check if `ranlib' utility is available
AC_PROG_RANLIB

dnl Check if C compiler supports `const' keyword
AC_C_CONST

dnl Check if the type `size_t' is defined
AC_TYPE_SIZE_T

dnl Check if the ANSI C headers are available
AC_HEADER_STDC
AC_CHECK_HEADERS([limits.h stdlib.h string.h unistd.h])

dnl Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([memset strchr strcspn strdup strerror])

dnl Check if the following optional headers are available
AC_CHECK_HEADERS(libgen.h getopt.h error.h)

dnl Check if the C library is GNU libc. If this is the case, then we can
dnl define _GNU_SOURCE in order to use the GNU C extensions.
AC_MSG_CHECKING(if C library is GNU libc)
AC_EGREP_CPP(GLIBC,
[
    #include <features.h>
    #ifdef __GLIBC__
        GLIBC
    #endif
],[
    AC_DEFINE(_GNU_SOURCE, [], [Define if you are using GNU libc])
    AC_MSG_RESULT(found)
],[
    AC_MSG_RESULT(no)
])

dnl Check if each of the following optional functions/variables are
dnl available. Normally the AC_REPLACE_FUNCS and AC_CONFIG_LIBOBJ_DIR
dnl macros are a more concise way of doing this this, but the source
dnl code filenames for each replacement function have to be truncated to
dnl MS-DOS 8.3 spec to allow for compiling under DJGPP (which certainly
dnl needs these substitutes).
AC_CHECK_FUNCS(getopt_long error program_invocation_short_name)

dnl Check if debugging assertions are to be enabled
AC_ARG_ENABLE(assert,
    AS_HELP_STRING(
        [--enable-assert],
        [Enable assertion checking code for debugging]),
    [enable_assert=$enableval],
    [enable_assert=no]
)
if test "$enable_assert" = no; then
    AC_DEFINE(NDEBUG,, [Define if compiling a production version])
fi

dnl Check if compiler optimisations should be disabled. Currently this
dnl only works for GCC; will need to look up the optimisation flags for
dnl other compilers such as Clang, Intel C compiler, etc.
AC_ARG_ENABLE(opt,
    AS_HELP_STRING(
        [--disable-opt],
        [Disable GCC compiler optimisations (default=enabled)]),
    [enable_opt=$enableval],
    [enable_opt=yes]
)
DISABLE_OPT_CFLAGS=""
if test "$enable_opt" = no; then
    dnl The shell variable $GCC will be set to "yes" if autoconf
    dnl recognises the system's compiler as GCC.
    if test "$GCC" = yes; then
        dnl This is intended to override the -O2 that autoconf normally
        dnl includes in CFLAGS.
        DISABLE_OPT_CFLAGS="-O0"
    fi
fi
AC_SUBST(DISABLE_OPT_CFLAGS)

dnl Check if compiler source code strictness should be disabled. As with
dnl --enable-opt, this currently only works for GCC; will need to look
dnl up strictness flags for other compilers. The idea behind this is
dnl that while the extra QA provided by the strictness flags should be
dnl enabled by default, it's possible for the end-user to disable it
dnl (should a future version of GCC become too anal and decides our code
dnl isn't kosher), or if they want to hack the codebase themselves and
dnl they want to target C99/C++ instead.
AC_ARG_ENABLE(strict,
    AS_HELP_STRING(
        [--enable-strict],
        [Enable GCC strict code validation (default=no)]),
    [enable_strict=$enableval],
    [enable_strict=no]
)
ENABLE_STRICT_CFLAGS=""
if test "$enable_strict" = yes; then
    dnl The shell variable $GCC will be set to "yes" if autoconf
    dnl recognises the system's compiler as GCC.
    dnl
    dnl Currently as of gcc 4.8.x, we'll have to disable the
    dnl -Woverlength-strings checking because Check's ck_assert() macros
    dnl are currently tripping it up. GCC claims that the macros produce
    dnl strings of 2000+ characters, even though manual inspection of
    dnl the pre-processor output only reveals strings of around 100
    dnl characters at most. Perhaps this is a bug that should be brought
    dnl to the GCC developers' attention?
    dnl
    if test "$GCC" = yes; then
        ENABLE_STRICT_CFLAGS="-Wall -Wpedantic -Wextra -Werror -Wno-overlength-strings -std=c90"
    fi
fi
AC_SUBST(ENABLE_STRICT_CFLAGS)

dnl Check what end-of-line sequence the host platform uses. The test
dnl program will write a file called `eol.tmp' which will contain a
dnl single line containing one of the strings "LF", "CR" or "CRLF". If
dnl cross-compiling, then default to LF.
dnl
dnl The option --enable-default-eol-mode=<mode> can be set to bypass this
dnl test if needed. The value of <mode> will appear in the shell variable
dnl $enable_default_eol_mode.
AC_ARG_ENABLE(default-eol-mode,
    AS_HELP_STRING(
        [--enable-default-eol-mode=MODE], [
    Override the default end-of-line character mode.
    MODE must be one of: lf crlf cr autodetect]),
    [enable_default_eol_mode=$enableval],
    [enable_default_eol_mode=autodetect]
)
case "$enable_default_eol_mode" in
    (lf)
        AC_DEFINE(DEFAULT_EOL_MODE, EM_LF, [
            Define to either EM_LF, EM_CR, or EM_CRLF to reflect the end-of-line
            character sequence that your system uses in text files.
        ])
    ;;
    (crlf)
        AC_DEFINE(DEFAULT_EOL_MODE, EM_CRLF)
    ;;
    (cr)
        AC_DEFINE(DEFAULT_EOL_MODE, EM_CR)
    ;;
    (autodetect|no)
        AC_MSG_CHECKING(end-of-line sequence used by target platform)
        AC_TRY_RUN(
        [
#include <stdio.h>
#include <stdlib.h>
int main()
{
    int status = EXIT_FAILURE;

    freopen("eol.tmp", "w", stdout);
    putchar(10);
    freopen("eol", "w", stdout);
    freopen("eol.tmp", "rb", stdin);
    switch(getchar())
    {
        case 10:
            puts("LF");
            status = EXIT_SUCCESS;
            break;
        case 13:
            switch(getchar())
            {
                case 10:
                    puts("CRLF");
                    status = EXIT_SUCCESS;
                    break;
                case EOF:
                    puts("CR");
                    status = EXIT_SUCCESS;
                    break;
            }
            break;
    }

    fclose(stdin);
    fclose(stdout);
    remove("eol.tmp");
    exit(status);
}
        ],[
            AC_DEFINE_UNQUOTED(DEFAULT_EOL_MODE, EM_`cat eol`)
            AC_MSG_RESULT([$(cat eol)])
        ],[
            AC_DEFINE(DEFAULT_EOL_MODE, EM_LF)
            AC_MSG_WARN([test program failed, assuming LF.
                Use --enable-default-eol-mode option to override.])
        ],[
            AC_MSG_WARN([cannot test when cross-compiling, defaulting to LF.
                Use --enable-default-eol-mode option to override.])
            AC_DEFINE(DEFAULT_EOL_MODE, EM_LF)
        ])
    ;;
    *)
        AC_MSG_ERROR([\
Argument to option `--enable-default-eol-mode' must be one of \
`lf', `cr', `crlf' or `autodetect': ${enable_default_eol_mode}])
    ;;
esac
rm -f eol

dnl Determine if pkg-config is available
PKG_PROG_PKG_CONFIG

dnl Determine if libcheck (unit testing framework) is available
PKG_CHECK_MODULES(CHECK, check,
[
    AC_SUBST([CHECK_CFLAGS])
    AC_SUBST([CHECK_LIBS])
],
[
    AC_MSG_NOTICE([

        Check <http://check.sourceforge.net/> is used for unit testing
        in this package.

        It is not a dependency for building ${PACKAGE_NAME} itself,
        however if you wish to run the automated test suite via `make
        check' prior to installation, you will need to obtain and
        install Check on your system.
    ])
])

dnl Generate makefiles for building the package
AC_OUTPUT(Makefile \
    compat/Makefile \
    tests/Makefile \
    tests/helpers/Makefile)

dnl Inform the user of completion, and tell them what to do next.
AC_MSG_NOTICE([

    Configuration of ${PACKAGE_NAME} is complete.

    Next, you'll need to compile the program source code. Please type:
        \$ make

    Optionally, you can compile and run the automated test suite*. Type:
        \$ make check
            (*requires Check <http://check.sourceforge.net/> unit test
            library installed.)

    Finally, to install* the executables/documentation on your system, type:
        \$ make install
            (*if installing into a system-wide path such as /usr or
            /usr/local, you'll need to run this command with super-user
            privileges.)

])
