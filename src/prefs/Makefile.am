#############
# libniftyled Makefile.am
# v0.4 - Daniel Hiepler <daniel@niftylight.de>


include $(top_srcdir)/src/Makefile.global.am

EXTRA_DIST = \
	_prefs_led.h \
	_prefs_chain.h \
	_prefs_hardware.h \
	_prefs_setup.h \
	_prefs_tile.h


# targets
noinst_LTLIBRARIES = libprefs.la

# sources
libprefs_la_SOURCES = \
	prefs.c \
	prefs_led.c \
	prefs_chain.c \
	prefs_hardware.c \
	prefs_setup.c \
	prefs_tile.c

# cflags
libprefs_la_CFLAGS = \
	$(COMMON_CFLAGS_N)

# linker flags
libprefs_la_LDFLAGS = \
	$(COMMON_LDFLAGS_N)
