## Process this file with automake to produce Makefile.in

# Note: Please try to keep source file names no longer than 8-chars max
# to accomodate testing with DJGPP under MS-DOS. Longer file names for
# derived objects are okay because we're only expecting GNU Autoconf to
# be used on UNIX-like platforms; use the standalone Makefile.djg
# makefile for building under DJGPP.

# Automake should recurse into these subdirectories in the given order
SUBDIRS = helpers .

# These programs will be built and run when "make check" is invoked.
TESTS = ckclnstr \
    ckflmgmt \
    ckoptns \
    ckprcfil \
    ckstrmio

# These are the unit-test suite programs to be built when "make check" is invoked.
check_PROGRAMS = ckclnstr \
    ckflmgmt \
    ckoptns \
    ckprcfil \
    ckstrmio

# Common compiler flags, linker flags and dependent libraries for each
# test suite program.
common_cflags = @CHECK_CFLAGS@
common_ldadd = @CHECK_LIBS@ \
    $(top_srcdir)/libcleantxt.a \
    $(top_srcdir)/tests/helpers/libhelpers.a \
    $(top_srcdir)/compat/libcompat.a
common_dependencies = $(top_srcdir)/libcleantxt.a \
    $(top_srcdir)/tests/helpers/libhelpers.a \
    $(top_srcdir)/compat/libcompat.a

ckclnstr_SOURCES = ckclnstr.c
ckclnstr_CFLAGS = $(common_cflags)
ckclnstr_LDADD = $(common_ldadd)
ckclnstr_DEPENDENCIES = $(common_dependencies)

ckflmgmt_SOURCES = ckflmgmt.c
ckflmgmt_CFLAGS = $(common_cflags)
ckflmgmt_LDADD = $(common_ldadd)
ckflmgmt_DEPENDENCIES = $(common_dependencies)

ckoptns_SOURCES = ckoptns.c
ckoptns_CFLAGS = $(common_cflags)
ckoptns_LDADD = $(common_ldadd)
ckoptns_DEPENDENCIES = $(common_dependencies)

ckprcfil_SOURCES = ckprcfil.c
ckprcfil_CFLAGS = $(common_cflags)
ckprcfil_LDADD = $(common_ldadd)
ckprcfil_DEPENDENCIES = $(common_dependencies)

ckstrmio_SOURCES = ckstrmio.c
ckstrmio_CFLAGS = $(common_cflags)
ckstrmio_LDADD = $(common_ldadd)
ckstrmio_DEPENDENCIES = $(common_dependencies)

# Allows for additional C flags that may be provided by configure's
# --disable-opt or --enable-strict arguments.
CFLAGS = @CFLAGS@ @DISABLE_OPT_CFLAGS@ @ENABLE_STRICT_CFLAGS@

# Derived files that should be deleted on `make maintainer-clean'
MAINTAINERCLEANFILES = \
    Makefile.in
