# libniftyled configure.ac 
# v0.3 - (c) Daniel Hiepler (2010-2012)

# --------------------------------
#    initialization
# --------------------------------

AC_INIT([libniftyled], m4_esyscmd([./version.sh --short]), [http://github.com/niftylight/niftyled/issues], [niftyled], [http://wiki.niftylight.de/AC_PACKAGE_NAME])
AC_COPYRIGHT([Copyright (C) 2006-2012 Daniel Hiepler])

AM_INIT_AUTOMAKE([color-tests dist-bzip2 parallel-tests silent-rules -Wall])
AC_CONFIG_MACRO_DIR([m4])


# --------------------------------
#    information on the package
# --------------------------------

# infos
PACKAGE_DESCRIPTION="library designed to provide an abstract interface for LED/lighting hardware to easily control it using pixel data."
AC_SUBST([PACKAGE_DESCRIPTION], [${PACKAGE_DESCRIPTION}], [package description])
AC_DEFINE_UNQUOTED(PACKAGE_DESCRIPTION, "${PACKAGE_DESCRIPTION}", [package description])

# versioning (s. version.sh)
PACKAGE_HW_PLUGIN_API_MAJOR_VERSION="m4_esyscmd([./version.sh --hw-major])"
PACKAGE_HW_PLUGIN_API_MINOR_VERSION="m4_esyscmd([./version.sh --hw-minor])"
PACKAGE_HW_PLUGIN_API_MICRO_VERSION="m4_esyscmd([./version.sh --hw-micro])"
AC_SUBST([PACKAGE_HW_PLUGIN_API_MAJOR_VERSION], [${PACKAGE_HW_PLUGIN_API_MAJOR_VERSION}], [hw-plugin API major version])
AC_SUBST([PACKAGE_HW_PLUGIN_API_MINOR_VERSION], [${PACKAGE_HW_PLUGIN_API_MINOR_VERSION}], [hw-plugin API minor version])
AC_SUBST([PACKAGE_HW_PLUGIN_API_MICRO_VERSION], [${PACKAGE_HW_PLUGIN_API_MICRO_VERSION}], [hw-plugin API micro version])
AC_DEFINE_UNQUOTED(PACKAGE_HW_PLUGIN_API_MAJOR_VERSION, "${PACKAGE_HW_PLUGIN_API_MAJOR_VERSION}", [hw-plugin API major version])
AC_DEFINE_UNQUOTED(PACKAGE_HW_PLUGIN_API_MINOR_VERSION, "${PACKAGE_HW_PLUGIN_API_MINOR_VERSION}", [hw-plugin API minor version])
AC_DEFINE_UNQUOTED(PACKAGE_HW_PLUGIN_API_MICRO_VERSION, "${PACKAGE_HW_PLUGIN_API_MICRO_VERSION}", [hw-plugin API micro version])
AC_SUBST([PACKAGE_MAJOR_VERSION], m4_esyscmd([./version.sh --major]), [major version])
AC_SUBST([PACKAGE_MINOR_VERSION], m4_esyscmd([./version.sh --minor]), [minor version])
AC_SUBST([PACKAGE_MICRO_VERSION], m4_esyscmd([./version.sh --micro]), [micro version])
AC_SUBST([PACKAGE_API_REVISION], m4_esyscmd([./version.sh --api-revision]), [api revision])
AC_SUBST([PACKAGE_API_CURRENT], m4_esyscmd([./version.sh --api-current]), [api current])
AC_SUBST([PACKAGE_API_AGE], m4_esyscmd([./version.sh --api-age]), [api age])
AC_SUBST([PACKAGE_GIT_VERSION], m4_esyscmd([./version.sh --git]), [git version])

#AC_DEFINE(GENERIC_SETTINGS_FILE_VERSION, SETTINGS_FILE_VERSION, [version of config-files])


# --------------------------------
#    checks for programs
# --------------------------------
LT_INIT
AM_PROG_CC_C_O
AM_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG([0.14])
if test -z "$PKG_CONFIG"; then
  AC_MSG_ERROR([*** pkg-config not found. See http://www.freedesktop.org/software/pkgconfig/ or check your distribution.])
fi
  

# --------------------------------
#    checks for libraries
# --------------------------------
PKG_CHECK_MODULES(niftylog, [niftylog >= 0.1], [], [AC_MSG_ERROR([You need libniftylog + development headers installed])])
AC_SUBST(niftylog_CFLAGS)
AC_SUBST(niftylog_LIBS)

PKG_CHECK_MODULES(niftyprefs, [niftyprefs >= 0.1], [], [AC_MSG_ERROR([You need libniftyprefs + development headers installed])])
AC_SUBST(niftyprefs_CFLAGS)
AC_SUBST(niftyprefs_LIBS)

PKG_CHECK_MODULES(babl, [babl >= 0.1.4], [], [AC_MSG_ERROR([You need libbabl + development headers installed])])
AC_SUBST(babl_CFLAGS)
AC_SUBST(babl_LIBS)


# --------------------------------
# Check for headers
# --------------------------------
AC_CHECK_HEADERS([dlfcn.h])
AC_CHECK_HEADERS([byteswap.h])


# --------------------------------
# Check for typesv
# --------------------------------


# --------------------------------
# Check for structures
# --------------------------------


# --------------------------------
# Check compiler characteristics
# --------------------------------
AC_C_BIGENDIAN


# --------------------------------
# Check for functions
# --------------------------------


# --------------------------------
# Check for system-services
# --------------------------------


# --------------------------------
# Check for target-specific stuff 
# when cross-compiling
# --------------------------------
case "${host}" in
    *-mingw32*|*-winnt*)
        CFLAGS="${CFLAGS} -DWIN32"
    ;;
esac


# --------------------------------
#   compile process
# --------------------------------
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])


# --------------------------------
# Arguments for configure-script
# --------------------------------
AC_ARG_ENABLE(
        debug,
	[  --disable-debug         Don't compile for debugging ],
	[ debug=false ],
	[ debug=true])
AM_CONDITIONAL(DEBUG, test x$debug = xtrue)				


AC_ARG_ENABLE(
	tests,
	[  --disable-tests         Don't build tests for "make check" ],
	[ tests=false ],
	[ tests=true])
AM_CONDITIONAL(RUN_TESTS, test x$tests = xtrue)


# --------------------------------
# Output
# --------------------------------
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Doxyfile \
	Makefile \
	include/Makefile \
	include/niftyled-version.h \
	src/version.c \
	src/Makefile \
	src/chain/Makefile \
	src/frame/Makefile \
	src/hardware/Makefile \
	src/prefs/Makefile \
	src/setup/Makefile \
	src/tile/Makefile \
	tests/Makefile \
	utils/Makefile \
	utils/ledset/Makefile \
	${PACKAGE}.pc \
	debian/control.tmpl
])
AC_OUTPUT


echo "
Configure report:

    Package.....................:  ${PACKAGE_STRING}
    Description.................:  ${PACKAGE_DESCRIPTION}
    Revision....................:  ${PACKAGE_GIT_VERSION}
    Libtool version.............:  ${PACKAGE_API_CURRENT}:${PACKAGE_API_REVISION}:${PACKAGE_API_AGE}
    URL.........................:  ${PACKAGE_URL}
    Bugreports..................:  ${PACKAGE_BUGREPORT}

    Install prefix..............:  ${prefix}
    C compiler..................:  ${CC}
    C++ compiler................:  ${CPP}
    System CFLAGS...............:  ${CFLAGS}
    System CXXFLAGS.............:  ${CXXFLAGS}
    System LDFLAGS..............:  ${LDFLAGS}
"
