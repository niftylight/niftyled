#############
# libniftyled Makefile.am
# v0.4 - Daniel Hiepler <daniel@niftylight.de>


include $(top_srcdir)/src/Makefile.global.am

EXTRA_DIST = \
        _relation.h \
        _thread.h

# targets
noinst_LTLIBRARIES = libutil.la

# sources
libutil_la_SOURCES = \
	relation.c \
	thread.c

# cflags
libutil_la_CFLAGS = \
	$(COMMON_CFLAGS_N)

# linker flags
libutil_la_LDFLAGS = \
	$(COMMON_LDFLAGS_N)
