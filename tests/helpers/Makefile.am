## Process this file with automake to produce Makefile.in

# Note: Please try to keep source file names no longer than 8-chars max
# to accomodate testing with DJGPP under MS-DOS. Longer file names for
# derived objects are okay because we're only expecting GNU Autoconf to
# be used on UNIX-like platforms; use the standalone Makefile.djg
# makefile for building under DJGPP.

# Compiler include paths; needed for locating config.h
INCLUDES = -I$(top_srcdir)

# Specifies source files needed to build the libhelpers.a library
libhelpers_a_SOURCES = hexcdiff.c io.c runner.c

# These libraries should only be built if `make check' is invoked.
check_LIBRARIES = libhelpers.a

MAINTAINERCLEANFILES = \
    Makefile.in

# Extra files that should be packaged up in the distribution archives
EXTRA_DIST = hexcdiff.h io.h

# Allows for additional C flags that may be provided by configure's
# --disable-opt or --enable-strict arguments.
CFLAGS = @CFLAGS@ @DISABLE_OPT_CFLAGS@ @ENABLE_STRICT_CFLAGS@
