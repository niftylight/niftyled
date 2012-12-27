#############
# libniftyled Makefile.am
# v0.4 - Daniel Hiepler <daniel@niftylight.de>

EXTRA_DIST = \
	tests.env

INCLUDE_DIRS = \
	-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	-I$(srcdir)

# custom cflags
WARN_CFLAGS = -Wall -Wextra -Werror -Wno-unused-parameter


TESTCFLAGS = \
	$(INCLUDE_DIRS) \
	$(WARN_CFLAGS) \
	$(niftyprefs_CFLAGS) \
	$(niftylog_CFLAGS) \
	$(babl_CFLAGS)

TESTLDFLAGS =

TESTLDADD = \
	$(top_builddir)/src/libniftyled.la \
	$(niftyprefs_LIBS) \
	$(niftylog_LIBS) \
	$(babl_LIBS)



check_PROGRAMS = mapping
TESTS = $(check_PROGRAMS)
TESTS_ENVIRONMENT = $(srcdir)/tests.env


mapping_SOURCES = mapping.c
mapping_CFLAGS = $(TESTCFLAGS)
mapping_LDFLAGS = $(TESTLDFLAGS)
mapping_LDADD = $(TESTLDADD)
