AC_PREREQ([2.60])
AC_INIT([InputProto], [2.3.2], [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg])
AM_INIT_AUTOMAKE([foreign dist-bzip2])

# Require xorg-macros: XORG_WITH_ASCIIDOC
m4_ifndef([XORG_MACROS_VERSION],
          [m4_fatal([must install xorg-macros 1.10 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.10)
XORG_DEFAULT_OPTIONS
XORG_ENABLE_SPECS
XORG_WITH_ASCIIDOC(8.4.5)

AC_CONFIG_FILES([Makefile
           specs/Makefile
           inputproto.pc])
AC_OUTPUT
