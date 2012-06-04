# libniftyled configure.ac 
# v0.2 - Daniel Hiepler (2010-2011)

# --------------------------------
# Initialization macros.
# --------------------------------

# naming
define([RELEASE_NAME], niftyled)
define([LIB_NAME], libniftyled)

#release versioning
define([MAJOR_VERSION], 0)
define([MINOR_VERSION], 1)
define([MICRO_VERSION], 1)
define([DATESTAMP], esyscmd(date +%Y%m%d | tr -d '\n\r'))
define([API_VERSION], 10000)
#                     |   |
#                     |   +-- increment by one if interfaces were added without 
#                     |       breaking the API (results in a warning)
#                     +------ increment by 10000 on API changes that break older versions
#                             (results in an error)

define([HW_PLUGIN_API_VERSION],   100002)
#                                 |  | |
#                                 |  | +-- increment by 1 (to 99) for micro 
#                                 |  |     changes (major changes that don't 
#                                 |  |     affect anything)
#                                 |  +---- increment by 100 (to 99900) for minor
#                                 |        changes (throws a warning, some 
#                                 |        functionality of some plugins might 
#                                 |        be affected)
#                                 +------- increment by 100000 for major changes 
#                                          (plugin load will fail, complete 
#                                          functionality of most/all plugins 
#                                          affected)

define([SETTINGS_FILE_VERSION],   000000)
#                                 |  | |
#                                 |  | +-- increment by 1 if on trivial changes 
#                                 |  |     of settings-file scheme (changes that
#                                 |  |      don't affect anything)
#                                 |  +---- increment by 100 on minor changes of 
#                                 |        settings-file scheme (throws a warning
#                                 |        that some functionality might be 
#                                 |        affected)
#                                 +------- increment by 100000 on major changes 
#                                          of settings-file scheme (will result
#                                          in fatal error when trying to load 
#                                          file)

# --------------------------------
# AC_INIT
# --------------------------------
AC_INIT([LIB_NAME],[MAJOR_VERSION.MINOR_VERSION.MICRO_VERSION],[https://github.com/niftylight/niftylight/issues], [RELEASE_NAME], [http://wiki.niftylight.de/LIB_NAME])
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION, AC_PACKAGE_BUGREPORT, AC_PACKAGE_URL)
AC_CONFIG_MACRO_DIR([m4])
AC_LANG([C])

# Test for new silent rules and enable only if they are available
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

                
# library name 
GENERIC_LIBRARY_NAME=RELEASE_NAME
# library version
GENERIC_LIBRARY_VERSION=MAJOR_VERSION:MINOR_VERSION
# package version
GENERIC_VERSION=MAJOR_VERSION.MINOR_VERSION.MICRO_VERSION
# release version
GENERIC_RELEASE=MAJOR_VERSION.MINOR_VERSION
# library API version
GENERIC_API_VERSION=API_VERSION
# plugin API version
GENERIC_HW_PLUGIN_API_VERSION=HW_PLUGIN_API_VERSION
# settings-file version
GENERIC_SETTINGS_FILE_VERSION=SETTINGS_FILE_VERSION




AC_DEFINE(GENERIC_MAJOR_VERSION, MAJOR_VERSION, [major version])
AC_DEFINE(GENERIC_MINOR_VERSION, MINOR_VERSION, [minor version])
AC_DEFINE(GENERIC_MICRO_VERSION, MICRO_VERSION, [micro version])
AC_DEFINE(GENERIC_DATESTAMP, DATESTAMP, [datestamp])
AC_DEFINE(GENERIC_API_VERSION, API_VERSION, [api version])
AC_DEFINE(GENERIC_HW_PLUGIN_API_VERSION, HW_PLUGIN_API_VERSION, [hardware plugin api version])
AC_DEFINE(GENERIC_SETTINGS_FILE_VERSION, SETTINGS_FILE_VERSION, [version of config-files])


# --------------------------------
# Check for programs
# --------------------------------
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL

PKG_PROG_PKG_CONFIG([0.14])
if test -z "$PKG_CONFIG"; then
  AC_MSG_ERROR([*** pkg-config not found. See http://www.freedesktop.org/software/pkgconfig/ or check your distribution.])
fi
  

# --------------------------------
# Check for libs
# --------------------------------
PKG_CHECK_MODULES(niftysettings, [niftysettings >= 0.1], [], [AC_MSG_ERROR([You need libniftysettings + development headers installed])])
AC_SUBST(niftysettings_CFLAGS)
AC_SUBST(niftysettings_LIBS)

PKG_CHECK_MODULES(babl, [babl >= 0.1.4], [], [AC_MSG_ERROR([You need libbabl + development headers installed])])
AC_SUBST(babl_CFLAGS)
AC_SUBST(babl_LIBS)


# --------------------------------
# Check for headers
# --------------------------------
AC_CONFIG_HEADERS([config.h])
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
# Arguments for configure-script
# --------------------------------
# enable debug
AC_ARG_ENABLE(
        debug,
        AS_HELP_STRING([--disable-debug],
                 [compile without debug code]),
        enable_debug=$enableval, enable_debug=yes )
if test x$enable_debug = xyes; then
  AC_DEFINE(ENABLE_DEBUG, [], [Enable debug code])
fi

# enable gprof
AC_ARG_ENABLE(
        gprof,
        AS_HELP_STRING([--enable-gprof=@<:@no/yes@:>@],
                [build with support for gprof]),,
        [enable_gprof=no])
if test "x$enable_gprof" != "xno" ; then
    CFLAGS="$CFLAGS -pg"
    LDFLAGS="$LDFLAGS -pg"
fi

AC_ARG_ENABLE(
	tests,
	[  --disable-tests         Don't run tests before install ],
	[ tests=false ],
	[ tests=true])
AM_CONDITIONAL(RUN_TESTS, test x$tests = xtrue)


# --------------------------------
#  special CFLAGS
# --------------------------------
WARNING_CFLAGS="-Wall -Wreturn-type -Wswitch -Wuninitialized -Wparentheses -Wpointer-arith -Winline"
EXTRA_CFLAGS="-std=c99"



AC_SUBST(GENERIC_SETTINGS_FILE_VERSION)
AC_SUBST(GENERIC_HW_PLUGIN_API_VERSION)
AC_SUBST(GENERIC_API_VERSION)
AC_SUBST(GENERIC_HW_PLUGIN_API_VERSION)
AC_SUBST(GENERIC_LIBRARY_VERSION)
AC_SUBST(GENERIC_LIBRARY_NAME)
AC_SUBST(GENERIC_RELEASE)
AC_SUBST(GENERIC_VERSION)
AC_SUBST(LIBDIR)
AC_SUBST(WARNING_CFLAGS)
AC_SUBST(EXTRA_CFLAGS)


# --------------------------------
# Output
# --------------------------------
AC_CONFIG_FILES([
	Makefile \
	include/Makefile \
	include/niftyled_version.h \
	src/Makefile \
	tests/Makefile \
        $GENERIC_LIBRARY_NAME.pc          
])
AC_OUTPUT

echo "
Configure summary:

        Version.....................:  AC_PACKAGE_VERSION
        API Version.................:  API_VERSION
        Build.......................:  DATESTAMP
	Compiler....................:  ${CC}
	Compiler Flags..............:  ${CFLAGS} ${ERROR_CFLAGS}
	Linker Flags................:  ${LDFLAGS}
	Prefix......................:  ${prefix}
"

