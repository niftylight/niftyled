#############
# libniftyled configure.ac
# v0.4 - (c) Daniel Hiepler (2010-2012)


# --------------------------------
#    initialization
# --------------------------------
AC_INIT([libniftyled], m4_esyscmd([./version --short]), [http://github.com/niftylight/niftyled/issues], [niftyled], [http://wiki.niftylight.de/AC_PACKAGE_NAME])
AC_COPYRIGHT([Copyright (C) 2006-2012 Daniel Hiepler])

AM_INIT_AUTOMAKE([color-tests dist-bzip2 parallel-tests silent-rules -Wall])
AC_CONFIG_MACRO_DIR([m4])


# --------------------------------
#    information on the package
# --------------------------------

# infos
PACKAGE_DESCRIPTION="library designed to provide an abstract interface for LED/lighting hardware"
PACKAGE_DESCRIPTION_LONG="library designed to provide an abstract interface for LED/lighting hardware to easily control it using pixel data."

AC_SUBST([PACKAGE_DESCRIPTION_LONG], [${PACKAGE_DESCRIPTION_LONG}], [long package description])
AC_SUBST([PACKAGE_DESCRIPTION], [${PACKAGE_DESCRIPTION}], [package description])
AC_DEFINE_UNQUOTED(PACKAGE_DESCRIPTION_LONG, "${PACKAGE_DESCRIPTION_LONG}", [long package description])
AC_DEFINE_UNQUOTED(PACKAGE_DESCRIPTION, "${PACKAGE_DESCRIPTION}", [package description])

# versioning (s. version)
PACKAGE_HW_PLUGIN_API_MAJOR_VERSION="m4_esyscmd([./version --hw-major])"
PACKAGE_HW_PLUGIN_API_MINOR_VERSION="m4_esyscmd([./version --hw-minor])"
PACKAGE_HW_PLUGIN_API_MICRO_VERSION="m4_esyscmd([./version --hw-micro])"
AC_SUBST([PACKAGE_HW_PLUGIN_API_MAJOR_VERSION], [${PACKAGE_HW_PLUGIN_API_MAJOR_VERSION}], [hw-plugin API major version])
AC_SUBST([PACKAGE_HW_PLUGIN_API_MINOR_VERSION], [${PACKAGE_HW_PLUGIN_API_MINOR_VERSION}], [hw-plugin API minor version])
AC_SUBST([PACKAGE_HW_PLUGIN_API_MICRO_VERSION], [${PACKAGE_HW_PLUGIN_API_MICRO_VERSION}], [hw-plugin API micro version])
AC_DEFINE_UNQUOTED(PACKAGE_HW_PLUGIN_API_MAJOR_VERSION, "${PACKAGE_HW_PLUGIN_API_MAJOR_VERSION}", [hw-plugin API major version])
AC_DEFINE_UNQUOTED(PACKAGE_HW_PLUGIN_API_MINOR_VERSION, "${PACKAGE_HW_PLUGIN_API_MINOR_VERSION}", [hw-plugin API minor version])
AC_DEFINE_UNQUOTED(PACKAGE_HW_PLUGIN_API_MICRO_VERSION, "${PACKAGE_HW_PLUGIN_API_MICRO_VERSION}", [hw-plugin API micro version])
AC_SUBST([PACKAGE_MAJOR_VERSION], m4_esyscmd([./version --major]), [major version])
AC_SUBST([PACKAGE_MINOR_VERSION], m4_esyscmd([./version --minor]), [minor version])
AC_SUBST([PACKAGE_MICRO_VERSION], m4_esyscmd([./version --micro]), [micro version])
AC_SUBST([PACKAGE_API_REVISION], m4_esyscmd([./version --api-revision]), [api revision])
AC_SUBST([PACKAGE_API_CURRENT], m4_esyscmd([./version --api-current]), [api current])
AC_SUBST([PACKAGE_API_AGE], m4_esyscmd([./version --api-age]), [api age])
AC_SUBST([PACKAGE_GIT_VERSION], m4_esyscmd([./version --git]), [git version])

#AC_DEFINE(GENERIC_SETTINGS_FILE_VERSION, SETTINGS_FILE_VERSION, [version of config-files])


# --------------------------------
#    checks for programs
# --------------------------------
LT_INIT
AM_PROG_CC_C_O
AM_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG([0.14])
if test -z "${PKG_CONFIG}"; then
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
#   compile process
# --------------------------------
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])


# --------------------------------
# Arguments for configure-script
# --------------------------------
AC_ARG_ENABLE(
        debug,
		AS_HELP_STRING([--enable-debug], [enable debugging, default: no]),
		[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
		esac],
		[debug=false])
AM_CONDITIONAL(DEBUG, test x$debug = xtrue)



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
	debian/control.tmpl \
	gentoo/media-gfx/niftyled/niftyled-9999.ebuild
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
