#############
# libniftyled Makefile.am
# v0.4 - Daniel Hiepler <daniel@niftylight.de>


include $(top_srcdir)/src/Makefile.global.am

EXTRA_DIST = _setup.h


# targets
noinst_LTLIBRARIES = libsetup.la

# sources
libsetup_la_SOURCES = \
	setup.c

# cflags
libsetup_la_CFLAGS = \
	$(COMMON_CFLAGS_N)

# linker flags
libsetup_la_LDFLAGS = \
	$(COMMON_LDFLAGS_N)
