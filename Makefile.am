## Process this file with GNU automake to produce Makefile.in

# Note: Please try to keep source file names no longer than 8-chars max
# to accomodate testing with DJGPP under MS-DOS. Longer file names for
# derived objects are okay because we're only expecting GNU Autoconf to
# be used on UNIX-like platforms; please use the standalone Makefile.djg
# makefile for building under DJGPP.

# Automake configuration options
#
# * "foreign" means only install minimal amount of support files
#   and that this project is not part of the official GNU system.
AUTOMAKE_OPTIONS = foreign

# Automake should recurse into these subdirectories in the given order
SUBDIRS = compat tests

# Compiler include options for building files in this directory
INCLUDES = -I$(top_srcdir)

# Tells automake the name of the main program
bin_PROGRAMS = cleantxt
# Sources pertaining exclusively to the main program
cleantxt_SOURCES = main.c
# Libraries that must be linked against to build the main program
cleantxt_LDADD = $(top_builddir)/libcleantxt.a \
    $(top_builddir)/compat/libcompat.a
# Main program must be re-linked if any of these files change
cleantxt_DEPENDENCIES = $(cleantxt_LDADD)

# Defines libraries that should be built in this directory, but not
# installed system-wide.
noinst_LIBRARIES = libcleantxt.a

# Defines the sources needed to build `libcleantxt.a'. We do this so the
# object code can be linked into both the main program and the test
# suites.
libcleantxt_a_SOURCES = options.c \
    cleanstr.c \
    filemgmt.c \
    procfile.c \
    streamio.c

# Extra files that should be packaged up in the distribution archives
EXTRA_DIST = Doxyfile \
    cleantxt.xml \
    cleantxt.1 \
    Makefile.djg \
    Makefile.lnx \
    Makefile.pkg \
    Makefile.rul \
    Makefile.dir \
    cleanstr.h \
    filemgmt.h \
    options.h \
    procfile.h \
    streamio.h \
    cleantxt.spec \
    README \
    TODO \
    COPYING \
    Changelog

# Man pages that will be installed system-wide during `make install'
man_MANS = cleantxt.1

# When the makefile target "make maintainer-clean" is run, these files
# should be removed to restore the source tree to a pristine state. Only
# non-derived files should remain.
#
# Note: config.status should be left out of this list, otherwise when
# the makefile tries to recurse it complains about this file being
# missing and aborts.
MAINTAINERCLEANFILES = \
    aclocal.m4 \
    config.h.in \
    config.guess \
    config.sub \
    config.log \
    configure \
    INSTALL \
    install-sh \
    Makefile.in \
    missing \
    mkinstalldirs \
    stamp-h.in

# Allows for additional C flags that may be provided by configure's
# --disable-opt or --enable-strict arguments.
CFLAGS = @CFLAGS@ @DISABLE_OPT_CFLAGS@ @ENABLE_STRICT_CFLAGS@

# The test suites are dependent on the non-main code in the top-level directory.
check: libcleantxt.a

# These makefile targets do not correspond to disk files
.PHONY: doxygen clean-doxygen man rm-autom4te.cache rm-man

# Require man pages to be built before packaging up the distribution
# archive; it's not reasonable to expect the end-user to have a working
# Docbook installation.
dist-hook: man

# Target for building the manpage from Docbook source
man: cleantxt.1

# Generates the man page using Docbook XML
cleantxt.1: cleantxt.xml
	docbook2man cleantxt.xml
	rm -rf manpage.links manpage.refs

# Generates Doxygen source documentation in the subdirectory "doxygen"
doxygen:
	doxygen

# Removes Doxygen source documentation
clean-doxygen:
	rm -rf doxygen

# Enables "make clean" to remove the generated Doxygen documentation
clean-am: clean-doxygen

# Additional directories that should be removed when "make
# maintainer-clean" target is invoked; automake doesn't do this by
# default.
maintainer-clean: rm-autom4te.cache rm-man

rm-autom4te.cache:
	rm -rf autom4te.cache

rm-man:
	rm -f cleantxt.1
