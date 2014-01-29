#############
# libniftyled Makefile.am
# v0.4 - Daniel Hiepler <daniel@niftylight.de>


include $(top_srcdir)/src/Makefile.global.am

EXTRA_DIST = _led.h


# targets
noinst_LTLIBRARIES = libled.la

# sources
libled_la_SOURCES = \
	led.c

# cflags
libled_la_CFLAGS = \
	$(COMMON_CFLAGS_N)

# linker flags
libled_la_LDFLAGS = \
	$(COMMON_LDFLAGS_N)
