#############
# libniftyled Makefile.am
# v0.4 - Daniel Hiepler <daniel@niftylight.de>


# AMFLAGS
ACLOCAL_AMFLAGS = -I m4


# subdirs to build
SUBDIRS = src include utils examples tests

# build documentation ?
if HAVE_DOXYGEN
    SUBDIRS += doc
endif


# files to include in archive
EXTRA_DIST = \
	debian \
	version \
	$(PACKAGE).pc.in

# files to clean on "make distclean"
DISTCLEANFILES = \
	$(PACKAGE).pc \
	-r doc/html \
	Doxyfile

# files to clean on "make maintainer-clean"
MAINTAINERCLEANFILES = \
	-r $(DEBTMPDIR)/


# pkgconfig for libnifty
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(PACKAGE).pc


# indent source & header-files
INDENT_C_ARGS=-pmt -bl -bls -cli8 -cbi0 -bli0 -cs -fca -i8 -sc -npsl -nut -npcs \
		-nsaf -nsai -cd2 -nce -ncdw -lc80 -nprs -nsaw -il0 -nbbo -bap \
		-hnl -lp -blf -ip0 -nbfda
INDENT_H_ARGS=-pmt -bl -bls -cli8 -cbi0 -bli0 -cs -fca -i8 -sc -npsl -nut -npcs \
		-nsaf -nsai -cd2 -nce -ncdw -l800 -lc800 -bad -nprs -nsaw \
		-di32 -hnl -nbc -nbfda -nbfde -T NftResult

.PHONY: indent
indent:
	@echo Indenting source-files...
	find $(top_srcdir)/tests $(top_srcdir)/include $(top_srcdir)/src -type f -and -name '*.[h]*' -not -empty -exec indent $(INDENT_H_ARGS) {} \;
	find $(top_srcdir)/tests $(top_srcdir)/src -type f -and -name '*.[c]*' -not -empty -exec indent $(INDENT_C_ARGS) {} \;


# create .deb package
# needs dpkg-dev, debhelper
DEBTMPDIR=$(abs_top_builddir)/deb-tmp
DEBSRCDIR=$(DEBTMPDIR)/@PACKAGE_NAME@-@PACKAGE_VERSION@
TARGZFILE=$(DEBTMPDIR)/@PACKAGE_NAME@@PACKAGE_API_REVISION@_@PACKAGE_VERSION@.orig.tar.gz

deb: dist-gzip
	@echo "Building Debian package..."; \
	rm --recursive --force "$(DEBTMPDIR)" ; \
	mkdir -p "$(DEBTMPDIR)" && \
	mv "$(top_builddir)/@PACKAGE@-@PACKAGE_VERSION@.tar.gz" "$(TARGZFILE)" && \
	tar --directory "$(DEBTMPDIR)/" --extract --gunzip --file "$(TARGZFILE)" && \
	mv "$(DEBTMPDIR)/@PACKAGE@-@PACKAGE_VERSION@" "$(DEBSRCDIR)" && \
	cp -r "$(top_builddir)/debian/"* "$(DEBSRCDIR)/debian" && \
	mv "$(DEBSRCDIR)/debian/rules.tmpl"                 "$(DEBSRCDIR)/debian/rules" && \
	mv "$(DEBSRCDIR)/debian/control.tmpl"               "$(DEBSRCDIR)/debian/control" && \
	mv "$(DEBSRCDIR)/debian/docbase"                    "$(DEBSRCDIR)/debian/@PACKAGE_NAME@-dev.doc-base" && \
	mv "$(DEBSRCDIR)/debian/@PACKAGE_NAME@.install"     "$(DEBSRCDIR)/debian/@PACKAGE_NAME@@PACKAGE_API_REVISION@.install" && \
	mv "$(DEBSRCDIR)/debian/@PACKAGE_NAME@.dirs"        "$(DEBSRCDIR)/debian/@PACKAGE_NAME@@PACKAGE_API_REVISION@.dirs" && \
	mv "$(DEBSRCDIR)/debian/@PACKAGE_NAME@-dbg.install" "$(DEBSRCDIR)/debian/@PACKAGE_NAME@@PACKAGE_API_REVISION@-dbg.install" && \
	mv "$(DEBSRCDIR)/debian/@PACKAGE_NAME@-dbg.dirs"    "$(DEBSRCDIR)/debian/@PACKAGE_NAME@@PACKAGE_API_REVISION@-dbg.dirs" && \
	mv "$(DEBSRCDIR)/debian/symbols"                    "$(DEBSRCDIR)/debian/@PACKAGE_NAME@@PACKAGE_API_REVISION@.symbols" && \
	cd "$(DEBSRCDIR)" && \
	dpkg-buildpackage -D
