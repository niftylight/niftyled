# libniftyled Makefile.am - v0.2 - Daniel Hiepler <daniel@niftylight.de> 

AUTOMAKE_OPTIONS = foreign 1.7 dist-bzip2
ACLOCAL_AMFLAGS = -I m4

# tools to use
XARGS = xargs
FIND = find
INDENT = indent

# subdirs to build
SUBDIRS = src include

# in order to set CFLAGS, set environment variable before compile
# CFLAGS = -O0 -g -Wall

# run checks? (configure argument "--disable-tests")
if RUN_TESTS
 SUBDIRS += tests
endif

# files to include in archive
EXTRA_DIST = Doxyfile $(GENERIC_LIBRARY_NAME).pc $(GENERIC_LIBRARY_NAME).pc.in .indent.pro
DISTCLEANFILES = $(GENERIC_LIBRARY_NAME).pc

# pkgconfig for libnifty
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(GENERIC_LIBRARY_NAME).pc




# --------------- convenience targets for developers

# indent source & header-files
.PHONY: indent
indent: $(top_srcdir)/.indent.pro
	@echo Indenting source-files...
	$(FIND) ./include -type f -and -name "*.[ch]" -not -empty -print | $(XARGS) $(top_srcdir)/missing --run $(INDENT)
	$(FIND) ./src -type f -and -name "*.[ch]" -not -empty -print | $(XARGS) $(top_srcdir)/missing --run $(INDENT)
