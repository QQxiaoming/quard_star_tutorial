#! /bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

# Test to make sure compiling Vala code really works with non-recursive make.

required="pkg-config valac gcc GNUmake"
. test-init.sh

mkdir src

cat >> 'configure.ac' << 'END'
AC_PROG_CC
AM_PROG_VALAC([0.7.0])
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0 >= 2.4])
AC_OUTPUT
END

cat > 'src/zardoz.vala' <<'END'
using GLib;

public class Zardoz {
  public static void main () {
    stdout.printf ("Zardoz!\n");
  }
}
END

cat > 'Makefile.am' <<'END'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = src/zardoz
src_zardoz_CFLAGS = $(GOBJECT_CFLAGS)
src_zardoz_LDADD = $(GOBJECT_LIBS)
src_zardoz_SOURCES = src/zardoz.vala
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE
test -f src/zardoz.c
test -f src_zardoz_vala.stamp
$MAKE distcheck
$MAKE maintainer-clean
test ! -e src/zardoz.c
test ! -e src_zardoz_vala.stamp

mkdir build
cd build
../configure
$MAKE
$MAKE distcheck

cd ..
rm -rf build

# Try again with subdir-objects.

cat > 'Makefile.am' <<'END'
AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = src/zardoz
src_zardoz_CFLAGS = $(GOBJECT_CFLAGS)
src_zardoz_LDADD = $(GOBJECT_LIBS)
src_zardoz_SOURCES = src/zardoz.vala
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure || skip_ "configure failure"
$MAKE
$MAKE distcheck
$MAKE distclean
mkdir build
cd build
../configure
$MAKE
$MAKE distcheck
