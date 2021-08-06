#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Test to make sure vala support handles from-scratch VPATH builds.
# See automake bug#8753.

required="cc valac pkg-config GNUmake"
. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_SRCDIR([hello.vala])
AC_PROG_CC
AM_PROG_VALAC([0.7.3])
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0 >= 2.4])
AC_OUTPUT
END

cat > Makefile.am <<'END'
bin_PROGRAMS = foo bar
AM_CFLAGS = $(GOBJECT_CFLAGS)
LDADD = $(GOBJECT_LIBS)
foo_SOURCES = hello.vala
bar_VALAFLAGS = -H zardoz.h
bar_SOURCES = goodbye.vala
END

cat > hello.vala <<'END'
void main ()
{
  stdout.printf ("foofoofoo\n");
}
END
cp hello.vala goodbye.vala

$ACLOCAL
$AUTOCONF
$AUTOMAKE

mkdir build
cd build
../configure
$MAKE
test -f ../foo_vala.stamp
test -f ../bar_vala.stamp
grep foofoofoo ../hello.c
test -f ../zardoz.h
$MAKE distcheck

# Rebuild rules work also in VPATH builds.

cat > ../hello.vala <<'END'
int main ()
{
  stdout.printf ("barbarbar\n");
  return 0;
}
END

$MAKE
test -f ../foo_vala.stamp
test -f ../bar_vala.stamp
grep barbarbar ../hello.c

# Rebuild rules are not uselessly triggered.
$MAKE -q
$MAKE -n | grep '\.stamp' && exit 1

# Cleanup rules work also in VPATH builds.
$MAKE clean
test -f ../foo_vala.stamp
test -f ../bar_vala.stamp
test -f ../zardoz.h
test -f ../hello.c
$MAKE maintainer-clean
test ! -e ../zardoz.h
test ! -e ../hello.c
test ! -e ../foo_vala.stamp
test ! -e ../bar_vala.stamp

:
