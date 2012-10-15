# libniftyled Makefile.am - v0.3 - Daniel Hiepler <daniel@niftylight.de> 

INCLUDE_DIRS = -I../include -I$(top_srcdir)/include
tests_CFLAGS_PRIV = -Wall $(INCLUDE_DIRS) $(niftyprefs_CFLAGS) $(niftylog_CFLAGS) $(babl_CFLAGS)
tests_LDFLAGS_PRIV =
tests_LDADD_PRIV = ../src/libniftyled.la $(niftyprefs_LIBS) $(niftylog_LIBS) $(babl_LIBS)



check_PROGRAMS = mapping
TESTS = $(check_PROGRAMS)


mapping_SOURCES = mapping.c
mapping_CFLAGS = $(tests_CFLAGS_PRIV)
mapping_LDFLAGS = $(tests_LDFLAGS_PRIV)
mapping_LDADD = $(tests_LDADD_PRIV)
