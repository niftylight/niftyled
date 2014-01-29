#############
# libniftyled configure.ac
# v0.5 - (c) Daniel Hiepler (2010-2013)


# --------------------------------
#    initialization
# --------------------------------

# package info
AC_INIT([libniftyled], m4_esyscmd([./version --short]), [http://github.com/niftylight/niftyled/issues], [niftyled], [http://wiki.niftylight.de/AC_PACKAGE_NAME])
AC_COPYRIGHT([Copyright (C) 2006-2013 Daniel Hiepler])

# for cross compiling
AC_CANONICAL_SYSTEM

# autotools initialization
AC_LANG(C)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([color-tests dist-bzip2 parallel-tests silent-rules -Wno-portability -Wall])



# --------------------------------
#    further info on the package
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
AC_SUBST([DEBIAN_REVISION], m4_esyscmd([./version --debian]), [debian revision])

#AC_DEFINE(GENERIC_SETTINGS_FILE_VERSION, SETTINGS_FILE_VERSION, [version of config-files])


# -----------------------------------------------
#    libtool stuff
# -----------------------------------------------
AM_PROG_AR
LT_INIT
AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL


# --------------------------------
#    checks for programs
# --------------------------------
dnl change this to 0.28 as soon as it's widely used. Then also delete m4/pkg.m4
PKG_PROG_PKG_CONFIG([0.25])
if test -z "${PKG_CONFIG}"; then
  	AC_MSG_ERROR([*** pkg-config not found. See http://www.freedesktop.org/software/pkgconfig/ or check your distribution.])
fi

AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -n "$DOXYGEN"; then 
  AC_CONFIG_FILES([debian/docbase])
fi
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])



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
AC_HEADER_STDC
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
AC_PROG_CC_C99
AM_PROG_CC_C_O
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
        Makefile 
        include/Makefile 
        include/niftyled-version.h 
        src/version.c 
        src/Makefile 
        src/util/Makefile
		src/led/Makefile		
        src/chain/Makefile 
        src/frame/Makefile 
        src/hardware/Makefile 
        src/prefs/Makefile 
        src/setup/Makefile 
        src/tile/Makefile 
        tests/Makefile 
        utils/Makefile 
        utils/ledset/Makefile 
        examples/Makefile 
        ${PACKAGE}.pc 
        doc/Doxyfile 
        doc/Makefile 
        debian/control.tmpl 
        debian/rules.tmpl
        debian/changelog
        debian/watch
        gentoo/media-gfx/niftyled/niftyled-9999.ebuild
])
AC_OUTPUT


echo -ne "
Configure report:

\tPackage.....................:  ${PACKAGE_STRING}
\tDescription.................:  ${PACKAGE_DESCRIPTION}
\tRevision....................:  ${PACKAGE_GIT_VERSION}
\tLibtool version.............:  ${PACKAGE_API_CURRENT}:${PACKAGE_API_REVISION}:${PACKAGE_API_AGE}
\tURL.........................:  ${PACKAGE_URL}
\tBugreports..................:  ${PACKAGE_BUGREPORT}

\tInstall prefix..............:  ${prefix}
\tC compiler..................:  ${CC}
\tSystem CFLAGS...............:  ${CFLAGS}
\tSystem CXXFLAGS.............:  ${CXXFLAGS}
\tSystem LDFLAGS..............:  ${LDFLAGS}
\tBuilding documentation......:  "
if test -n "${DOXYGEN}" ; then echo "yes" ; else echo "no" ; fi
