# libniftyled Makefile.am - v0.2 - Daniel Hiepler <daniel@niftylight.de> 

# AMFLAGS
ACLOCAL_AMFLAGS = -I m4


# flags for all objects
AM_CFLAGS = 
AM_CPPFLAGS =
AM_LIBS = 
AM_LDFLAGS =


# subdirs to build
SUBDIRS = src include

# compile tests for "make check"? (configure argument "--disable-tests")
if RUN_TESTS
    SUBDIRS += tests
endif

# files to include in archive
EXTRA_DIST = Doxyfile Doxyfile.in $(PACKAGE).pc $(PACKAGE).pc.in
DISTCLEANFILES = Doxyfile $(PACKAGE).pc

# pkgconfig for libnifty
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(PACKAGE).pc
