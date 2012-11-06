#############
# libniftyled Makefile.am
# v0.4 - Daniel Hiepler <daniel@niftylight.de>


include Makefile.global.am


# module-subdirs to include
SUBDIRS = \
	chain \
	frame \
	hardware \
	prefs \
	setup \
	tile

EXTRA_DIST = Makefile.global.am


# target library
lib_LTLIBRARIES = lib@PACKAGE@.la

# sources
lib@PACKAGE@_la_SOURCES = \
	version.c

# cflags
lib@PACKAGE@_la_CFLAGS = \
	$(COMMON_CFLAGS_N) \
	-DPACKAGE_GIT_VERSION="\"`$(top_srcdir)/version --git`\""

# linker flags
lib@PACKAGE@_la_LDFLAGS = \
	-version-info @PACKAGE_API_CURRENT@:@PACKAGE_API_REVISION@:@PACKAGE_API_AGE@ \
	-release @PACKAGE_MAJOR_VERSION@.@PACKAGE_MINOR_VERSION@ \
	$(COMMON_LDFLAGS_N) \
	$(niftylog_LIBS) \
	$(niftyprefs_LIBS) \
	$(babl_LIBS)

# link in modules from subdirectories @todo@ dynamic?
lib@PACKAGE@_la_LIBADD = \
	-lm -ldl \
	$(SUBDIRS) \
	$(niftyprefs_LIBS) \
	$(babl_LIBS) \
	chain/libchain.la \
	frame/libframe.la \
	hardware/libhardware.la \
	prefs/libprefs.la \
	setup/libsetup.la \
	tile/libtile.la
