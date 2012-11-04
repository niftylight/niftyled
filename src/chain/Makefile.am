#############
# libniftyled Makefile.am
# v0.4 - Daniel Hiepler <daniel@niftylight.de>


include $(top_srcdir)/src/Makefile.global.am

EXTRA_DIST = _chain.h


# targets
noinst_LTLIBRARIES = libchain.la

# sources
libchain_la_SOURCES = \
	chain.c

# cflags
libchain_la_CFLAGS = \
	$(COMMON_CFLAGS_N)

# linker flags
libchain_la_LDFLAGS = \
	$(COMMON_LDFLAGS_N)
