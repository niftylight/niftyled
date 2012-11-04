#############
# libniftyled Makefile.am
# v0.4 - Daniel Hiepler <daniel@niftylight.de>


include $(top_srcdir)/src/Makefile.global.am

EXTRA_DIST = _hardware.h


# targets
noinst_LTLIBRARIES = libhardware.la

# sources
libhardware_la_SOURCES = \
	hardware.c

# cflags
libhardware_la_CFLAGS = \
	$(COMMON_CFLAGS_N)

# linker flags
libhardware_la_LDFLAGS = \
	$(COMMON_LDFLAGS_N)
