#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Building libraries (libtool and static) from Vala sources.
# And use of vapi files to call C code from Vala.

required="valac cc pkg-config libtoolize GNUmake"
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_PROG_LIBTOOL
AM_PROG_VALAC([0.7.3])
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0 >= 2.4])
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
lib_LIBRARIES = libmu.a
lib_LTLIBRARIES = src/libzardoz.la
libmu_a_SOURCES = mu.vala mu2.c mu.vapi mu2.h
libmu_a_CPPFLAGS = -DOKOKIMDEFINED=1
libmu_a_VALAFLAGS = --vapidir=$(srcdir)
AM_CFLAGS = $(GOBJECT_CFLAGS)
src_libzardoz_la_LIBADD = $(GOBJECT_LIBS)
src_libzardoz_la_SOURCES = src/zardoz-foo.vala src/zardoz-bar.vala
src/zardoz-bar.vala: src/zardoz-foo.vala
	sed 's/Foo/Bar/g' $< >$@
END

libtoolize
$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

cat > mu2.c << 'END'
#include "mu2.h"
int mu2 (void)
{
  return OKOKIMDEFINED;
}
END

cat > mu2.h << 'END'
int mu2 (void);
END

cat > mu.vapi <<'END'
[CCode (cheader_filename = "mu2.h", cname = "mu2")]
public int c_mu2 ();
END

cat > mu.vala << 'END'
int main ()
{
  stdout.printf ("mumumu\n");
  return c_mu2 ();
}
END

mkdir -p src
cat > src/zardoz-foo.vala << 'END'
using GLib;
public class Foo {
  public static void zap () {
    stdout.printf ("FooFooFoo!\n");
  }
}
END

$MAKE
test -f libmu.a
test -f src/libzardoz.la
$FGREP "mumumu" mu.c
$FGREP "FooFooFoo" src/zardoz-foo.c
$FGREP "BarBarBar" src/zardoz-bar.c
test -f libmu_a_vala.stamp
test -f src_libzardoz_la_vala.stamp

$MAKE distcheck

:
