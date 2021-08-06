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

# Vala support with parallel make.

required='valac cc pkg-config GNUmake'
. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AC_PROG_CXX
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0 >= 2.4])
AM_PROG_VALAC([0.7.3])
AC_OUTPUT
END

cat > Makefile.am <<'END'
bin_PROGRAMS = zardoz
AM_CFLAGS = $(GOBJECT_CFLAGS)
LDADD = $(GOBJECT_LIBS)
zardoz_SOURCES = main.vala 1.vala 2.vala 3.vala 4.vala 5.vala 6.vala
END

echo 'int main () {' > main.vala
for i in 1 2 3 4 5 6; do
  echo "void foo$i () { stdout.printf (\"ok $i\\n\"); }" > $i.vala
  echo "foo$i ();" >> main.vala
done
echo 'return 0; }' >> main.vala

cat main.vala # For debugging.

$ACLOCAL
$AUTOMAKE -a
$AUTOCONF

./configure

$MAKE -j3
ls -l # For debugging.
for x in main 1 2 3 4 5 6; do test -f $x.c; done
test -f  zardoz_vala.stamp

$MAKE maintainer-clean -j4
ls -l # For debugging.
for x in main 1 2 3 4 5 6; do test ! -e $x.c; done
test ! -e zardoz_vala.stamp

mkdir build
cd build
../configure
$MAKE -j6
ls -l . .. # For debugging.
for x in main 1 2 3 4 5 6; do test -f ../$x.c; done
test -f ../zardoz_vala.stamp

$MAKE distcheck -j4

$MAKE maintainer-clean -j2
for x in main 1 2 3 4 5 6; do test ! -e ../$x.c; done
test ! -e ../zardoz_vala.stamp

:
