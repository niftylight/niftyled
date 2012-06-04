# libnifty Makefile.am - v0.2 - Daniel Hiepler <daniel@niftylight.de> 

INCLUDE_DIRS = -I../include -I$(top_srcdir)/include


# module-subdirs to include
SUBDIRS =
EXTRA_DIST = 

# target library
lib_LTLIBRARIES = lib@GENERIC_LIBRARY_NAME@.la

# source-files
lib@GENERIC_LIBRARY_NAME@_la_SOURCES = \
	version.c \
	tile.c \
	chain.c \
	hardware.c \
	frame.c \
	pixel_format.c \
	fps.c \
	settings.c \
	settings_tile.c \
	settings_chain.c \
	settings_hardware.c



# cflags 
lib@GENERIC_LIBRARY_NAME@_la_CFLAGS = \
        $(EXTRA_CFLAGS) \
	$(WARNING_CFLAGS) \
	$(INCLUDE_DIRS) \
	-DPLUGINDIR=\"@GENERIC_LIBRARY_NAME@-plugins\" \
        -D_BSD_SOURCE \
	$(niftysettings_CFLAGS) \
	$(babl_CFLAGS)

# linker flags
lib@GENERIC_LIBRARY_NAME@_la_LDFLAGS = \
	-version-info $(GENERIC_LIBRARY_VERSION) \
	-release $(GENERIC_RELEASE) \
	-ldl \
        -Wall -no-undefined

# link in modules from subdirectories
lib@GENERIC_LIBRARY_NAME@_la_LIBADD = \
	$(niftysettings_LIBS) \
	$(babl_LIBS) \
	-lm
