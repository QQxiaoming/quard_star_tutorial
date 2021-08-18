
# Initialize Autoconf
AC_PREREQ([2.60])
AC_INIT([libICE], [1.0.10],
        [https://gitlab.freedesktop.org/xorg/lib/libICE/issues], [libICE])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([config.h])

# Set common system defines for POSIX extensions, such as _GNU_SOURCE
# Must be called before any macros that run the compiler (like
# AC_PROG_LIBTOOL or XORG_DEFAULT_OPTIONS) to avoid autoconf errors.
AC_USE_SYSTEM_EXTENSIONS

# Initialize Automake
AM_INIT_AUTOMAKE([foreign dist-bzip2])

# Initialize libtool
AC_PROG_LIBTOOL

# Require xorg-macros minimum of 1.12 for DocBook external references
m4_ifndef([XORG_MACROS_VERSION],
          [m4_fatal([must install xorg-macros 1.12 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.12)
XORG_DEFAULT_OPTIONS
XORG_ENABLE_DOCS
XORG_ENABLE_SPECS
XORG_WITH_XMLTO(0.0.22)
XORG_WITH_FOP
XORG_WITH_XSLTPROC
XORG_CHECK_SGML_DOCTOOLS(1.8)

# Obtain compiler/linker options for depedencies
PKG_CHECK_MODULES(ICE, xproto xtrans)

# Transport selection macro from xtrans.m4
XTRANS_CONNECTION_FLAGS
AC_DEFINE(ICE_t, 1, [Xtrans transport type])
AC_DEFINE(TRANS_CLIENT, 1, [Xtrans transport client code])
AC_DEFINE(TRANS_SERVER, 1, [Xtrans transport server code])

# Checks for library functions.
AC_CHECK_LIB([bsd], [arc4random_buf])
AC_CHECK_FUNCS([asprintf arc4random_buf getentropy])

# Allow checking code with lint, sparse, etc.
XORG_WITH_LINT
XORG_LINT_LIBRARY([ICE])
LINT_FLAGS="${LINT_FLAGS} ${ICE_CFLAGS} ${XTRANS_CFLAGS}"

AC_CONFIG_FILES([Makefile
		doc/Makefile
		specs/Makefile
		src/Makefile
		ice.pc])
AC_OUTPUT
