# libniftyled Makefile.am - v0.3 - Daniel Hiepler <daniel@niftylight.de> 


include Makefile.global.am


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
	$(COMMON_CFLAGS_N)

# linker flags
lib@PACKAGE@_la_LDFLAGS = \
	-version-info @PACKAGE_API_CURRENT@:@PACKAGE_API_REVISION@:@PACKAGE_API_AGE@ \
	-release @PACKAGE_MAJOR_VERSION@.@PACKAGE_MINOR_VERSION@.@PACKAGE_MICRO_VERSION@ \
	-ldl \
	$(COMMON_LDFLAGS_N)

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
