#############
# libniftylog Makefile.am
# v0.4 - Daniel Hiepler <daniel@niftylight.de>


htmldir = \
        $(docdir)/html

html_DATA = \
        html/*


DISTCLEANFILES = \
	Doxyfile

CLEANFILES = \
	doxyfile.stamp \
	-r html


html/*:
	$(DOXYGEN) Doxyfile

doxyfile.stamp: html/*
	echo stamp > doxyfile.stamp

all-local: doxyfile.stamp
