# libniftyled Makefile.am - v0.3 - Daniel Hiepler <daniel@niftylight.de> 

INCLUDE_DIRS = \
	-I../include \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/chain \
	-I$(top_srcdir)/src/frame \
	-I$(top_srcdir)/src/hardware \
	-I$(top_srcdir)/src/prefs \
	-I$(top_srcdir)/src/setup \
	-I$(top_srcdir)/src/tile

# module-subdirs to include
SUBDIRS =
EXTRA_DIST = 

# source-files
lib@PACKAGE@_la_SOURCES = \
	version.c \
	tile/tile.c \
	chain/chain.c \
	hardware/hardware.c \
	setup/setup.c \
	frame/frame.c \
	frame/pixel_format.c \
	frame/fps.c \
	prefs/prefs.c \
	prefs/prefs_setup.c \
	prefs/prefs_hardware.c \
	prefs/prefs_tile.c \
	prefs/prefs_chain.c



# compile for debugging ?
if DEBUG
    OPT_CFLAGS = -O0 -g
else
    OPT_CFLAGS = -O2
endif


# target library
lib_LTLIBRARIES = lib@PACKAGE@.la


# cflags 
lib@PACKAGE@_la_CFLAGS = \
	-Wall \
	-D_BSD_SOURCE \
	-DPLUGINDIR=\"@PACKAGE@-plugins\" \
	-DPACKAGE_GIT_VERSION="\"`$(top_srcdir)/version.sh --git`\"" \
	$(OPT_CFLAGS) \
	$(INCLUDE_DIRS) \
	$(niftyprefs_CFLAGS) \
	$(babl_CFLAGS) 

# linker flags
lib@PACKAGE@_la_LDFLAGS = \
	-version-info @PACKAGE_API_CURRENT@:@PACKAGE_API_REVISION@:@PACKAGE_API_AGE@ \
	-release @PACKAGE_MAJOR_VERSION@.@PACKAGE_MINOR_VERSION@.@PACKAGE_MICRO_VERSION@ \
	-ldl \
	-Wall -no-undefined

# link in modules from subdirectories
lib@PACKAGE@_la_LIBADD = $(niftyprefs_LIBS) $(babl_LIBS) -lm
