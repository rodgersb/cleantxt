## Process this file with GNU automake to produce Makefile.in

# Note: Please try to keep source file names no longer than 8-chars max
# to accomodate testing with DJGPP under MS-DOS. Longer file names for
# derived objects are okay because we're only expecting GNU Autoconf to
# be used on UNIX-like platforms; please use the standalone Makefile.djg
# makefile for building under DJGPP.

# Compiler include paths; needed for locating config.h
INCLUDES = -I$(top_srcdir)

# Defines the libraries to build in this directory and that they should not
# be installed system-wide.
noinst_LIBRARIES = libcompat.a

# Specifies the source files needed to build libcompat.a
libcompat_a_SOURCES = error.c goptlong.c progname.c

# Derived files that should be deleted on `make maintainer-clean'
MAINTAINERCLEANFILES = \
    Makefile.in

# Extra files that should be packaged up in the distribution archives
EXTRA_DIST = error.h goptlong.h progname.h Makefile.dir

# Allows for additional C flags that may be provided by configure's
# --disable-opt or --enable-strict arguments.
CFLAGS = @CFLAGS@ @DISABLE_OPT_CFLAGS@ @ENABLE_STRICT_CFLAGS@
