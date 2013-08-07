#############
# libniftylog Makefile.am
# v0.4 - Daniel Hiepler <daniel@niftylight.de>


DISTCLEANFILES = \
	Doxyfile

CLEANFILES = \
	doxyfile.stamp \
	-r html


doxyfile.stamp:
	$(DOXYGEN) Doxyfile
	echo stamp > doxyfile.stamp

all-local: doxyfile.stamp
