# libniftyled Makefile.am - v0.3 - Daniel Hiepler <daniel@niftylight.de> 


# compile for debugging ?
if DEBUG
    OPT_CFLAGS = -O0 -g
else
    OPT_CFLAGS = -O2
endif

# include dirs
INCLUDE_DIRS = \
	-I$(abs_top_srcdir)/include \
	-I$(abs_top_srcdir)/src \
	-I$(abs_top_srcdir)/src/chain \
	-I$(abs_top_srcdir)/src/frame \
	-I$(abs_top_srcdir)/src/hardware \
	-I$(abs_top_srcdir)/src/prefs \
	-I$(abs_top_srcdir)/src/setup \
	-I$(abs_top_srcdir)/src/tile

# global cflags
export COMMON_CFLAGS = \
	-Wall \
	-D_BSD_SOURCE \
	-DPLUGINDIR=\"@PACKAGE@-plugins\" \
	$(OPT_CFLAGS) \
	$(niftyprefs_CFLAGS) \
	$(babl_CFLAGS) \
	$(INCLUDE_DIRS)

# global ldflags
export COMMON_LDFLAGS = \
	-Wall -no-undefined -no-allow-shlib-undefined

# module-subdirs to include
SUBDIRS = \
	chain \
	frame \
	hardware \
	prefs \
	setup \
	tile

EXTRA_DIST = 

# object files
lib@PACKAGE@_la_SOURCES = \
	version.c
	


# target library
lib_LTLIBRARIES = lib@PACKAGE@.la

# cflags 
lib@PACKAGE@_la_CFLAGS = \
	-DPACKAGE_GIT_VERSION="\"`$(top_srcdir)/version.sh --git`\"" \
	$(COMMON_CFLAGS)

# linker flags
lib@PACKAGE@_la_LDFLAGS = \
	-version-info @PACKAGE_API_CURRENT@:@PACKAGE_API_REVISION@:@PACKAGE_API_AGE@ \
	-release @PACKAGE_MAJOR_VERSION@.@PACKAGE_MINOR_VERSION@.@PACKAGE_MICRO_VERSION@ \
	-ldl \
	$(COMMON_LDFLAGS)

# link in modules from subdirectories
lib@PACKAGE@_la_LIBADD = \
	$(niftyprefs_LIBS) \
	$(babl_LIBS) \
	-lm \
	tile/libtile.la \
	chain/libchain.la \
	hardware/libhardware.la \
	setup/libsetup.la \
	frame/libframe.la \
	prefs/libprefs.la
