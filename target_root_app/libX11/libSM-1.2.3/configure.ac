
# Initialize Autoconf
AC_PREREQ(2.60)
AC_INIT([libSM], [1.2.3],
        [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg], [libSM])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([config.h])

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
XORG_WITH_XMLTO(0.0.22)
XORG_WITH_FOP
XORG_WITH_XSLTPROC
XORG_CHECK_SGML_DOCTOOLS(1.8)

# Obtain compiler/linker options for depedencies
PKG_CHECK_MODULES(SM, [ice >= 1.0.5] xproto xtrans)

# Needed to check for TCP & IPv6 support and set flags appropriately
XTRANS_CONNECTION_FLAGS

AC_ARG_WITH(libuuid, AS_HELP_STRING([--with-libuuid], [Build with libuuid support for client IDs]))

AC_CHECK_FUNCS([uuid_create], [], [
    if test x"$with_libuuid" != xno && test x"$have_system_uuid" != xyes; then
        PKG_CHECK_MODULES(LIBUUID, uuid, [HAVE_LIBUUID=yes], [HAVE_LIBUUID=no])
    fi
    if test x"$with_libuuid" = xyes && test x"$HAVE_LIBUUID" = xno; then
        AC_MSG_ERROR([requested libuuid support but uuid.pc not found])
    fi
])

AM_CONDITIONAL(WITH_LIBUUID, test x"$HAVE_LIBUUID" = xyes)

AC_CONFIG_FILES([Makefile
		doc/Makefile
		src/Makefile
		sm.pc])
AC_OUTPUT
