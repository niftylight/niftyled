# libniftyled Makefile.am - v0.3 - Daniel Hiepler <daniel@niftylight.de> 

# AMFLAGS
ACLOCAL_AMFLAGS = -I m4


# flags for all objects
AM_CFLAGS = 
AM_CPPFLAGS =
AM_LIBS = 
AM_LDFLAGS =


# subdirs to build
SUBDIRS = src include utils

# compile tests for "make check"? (configure argument "--disable-tests")
if RUN_TESTS
    SUBDIRS += tests
endif

# files to include in archive
EXTRA_DIST = \
	debian \
	version.sh \
	$(PACKAGE).pc.in

DISTCLEANFILES = \
	Doxyfile \
	$(PACKAGE).pc

# pkgconfig for libnifty
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(PACKAGE).pc


# create .deb package
DEBTMPDIR=deb-tmp
DEBSRCDIR=$(DEBTMPDIR)/$(PACKAGE_NAME)-$(VERSION)-$(PACKAGE_API_REVISION)
deb: dist-gzip
	@echo "Building Debian package..."; \
	rm --recursive --force "$(DEBTMPDIR)" ; \
	if ! [ -d debian ] ; then \
	    echo "The files needed for building a Debian package are not" \
		"included by default in the distribution. To build a package, check" \
		"out the project from source control."; \
	    exit 1; \
	fi; \
	mkdir -p "$(DEBTMPDIR)" && \
	mv "$(srcdir)/$(PACKAGE)-$(VERSION).tar.gz" "$(DEBTMPDIR)/$(PACKAGE)_$(VERSION).orig.tar.gz" && \
	tar --directory "$(DEBTMPDIR)/" --extract --gunzip --file "$(DEBTMPDIR)/$(PACKAGE)_$(VERSION).orig.tar.gz" && \
	mv "$(DEBTMPDIR)/$(PACKAGE)-$(VERSION)" "$(DEBSRCDIR)" && \
	cp -r "$(srcdir)/debian/"* "$(DEBSRCDIR)/debian" && \
	cp "$(srcdir)/debian/control.tmpl" "$(DEBSRCDIR)/debian/control" && \
	cd "$(DEBSRCDIR)" && \
	debuild -D -v"$(VERSION)-$(PACKAGE_API_REVISION)"
