#############
# libniftyled Makefile.am
# v0.4 - Daniel Hiepler <daniel@niftylight.de>


# directories to include
INCLUDE_DIRS = \
	-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	-I$(srcdir)

# custom cflags
WARN_CFLAGS = -Wall -Wextra -Werror -Wno-unused-parameter -Wno-missing-field-initializers


bin_PROGRAMS = ledset


# sources
ledset_SOURCES = \
	ledset.c


# compile for debugging ?
if DEBUG
    DEBUG_CFLAGS = -DDEBUG
endif


ledset_CFLAGS = \
	$(INCLUDE_DIRS) \
	$(DEBUG_CFLAGS) \
	$(WARN_CFLAGS) \
	$(niftyprefs_CFLAGS) \
	$(niftylog_CFLAGS) \
	$(babl_CFLAGS)

ledset_LDADD = \
	$(top_builddir)/src/libniftyled.la \
	$(niftyprefs_LIBS) \
	$(niftylog_LIBS) \
	$(babl_LIBS)

ledset_LDFLAGS = \
	-Wall -no-undefined -pthread
