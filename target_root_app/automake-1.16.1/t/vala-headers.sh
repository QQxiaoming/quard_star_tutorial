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

# Test to make sure compiling Vala code really works with recursive make.

required="pkg-config valac gcc GNUmake"
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_VALAC([0.7.0])
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0 >= 2.4])
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = zardoz quux

zardoz_SOURCES = zardoz.vala
quux_SOURCES = quux.vala
quux.vala: zardoz.vala
	sed 's/Zardoz/Quux/' <zardoz.vala >quux.vala

quux_VALAFLAGS = \
  --header HDR.h \
  --vapi hello.vapi

zardoz_VALAFLAGS = \
  -H foo.h \
  --internal-header foo2.h \
  --internal-vapi foo3.vapi

AM_CFLAGS = $(GOBJECT_CFLAGS)
LDADD = $(GOBJECT_LIBS)
END

headers='HDR.h hello.vapi foo.h foo2.h foo3.vapi'

cat > zardoz.vala << 'END'
using GLib;
public class Zardoz {
  public static void main () {
    stdout.printf ("Zardoz!\n");
  }
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE

# Test rebuild rules.

for h in $headers; do
  rm -f $h
  $MAKE $h
  test -f $h
done

rm -f $headers
$MAKE $headers
for h in $headers; do test -f $h; done

$MAKE distcheck

$MAKE maintainer-clean
for h in $headers; do test ! -e $h; done

:
