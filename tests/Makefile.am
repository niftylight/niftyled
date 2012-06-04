INCLUDE_DIRS = -I../include -I$(top_srcdir)/include

check_PROGRAMS = mapping

# test-target
TESTS = $(check_PROGRAMS)

my_cflags = $(INCLUDE_DIRS) -Wall $(niftysettings_CFLAGS) $(babl_CFLAGS)
my_ldflags = ../src/libniftyled.la $(niftysettings_LIBS) $(babl_LIBS)

mapping_SOURCES = mapping.c
mapping_CFLAGS = $(my_cflags)
mapping_LDADD = $(my_ldflags)
