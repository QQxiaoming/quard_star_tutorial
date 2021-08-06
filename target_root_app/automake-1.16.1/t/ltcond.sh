#!/bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Test for conditional libtool libraries.
# This combines two examples from the manual.

required='cc libtoolize'
. test-init.sh

cat >>configure.ac <<'END'
AM_CONDITIONAL([WANT_LIBFOO], [true])
AM_CONDITIONAL([WANT_LIBBAR], [false])
AC_SUBST([WANTEDLIBS], ['lib1foo.la lib1bar.la'])
AC_PROG_CC
AM_PROG_AR
AC_PROG_LIBTOOL
AC_OUTPUT
END

cat >Makefile.am <<'END'
EXTRA_LTLIBRARIES = lib1foo.la lib1bar.la lib3bar.la
lib_LTLIBRARIES = $(WANTEDLIBS)
lib1foo_la_SOURCES = foo.c
lib1foo_la_LDFLAGS = -rpath '$(libdir)'
lib1bar_la_SOURCES = bar.c
lib1bar_la_LDFLAGS = -rpath '$(libdir)'
lib3bar_la_SOURCES = bar.c

if WANT_LIBFOO
lib_LTLIBRARIES += lib2foo.la
check_LTLIBRARIES = lib3foo.la
endif
if WANT_LIBBAR
lib_LTLIBRARIES += lib2bar.la
endif
lib2foo_la_SOURCES = foo.c
lib2bar_la_SOURCES = bar.c
lib3foo_la_SOURCES = foo.c
END

echo 'int one () { return 1; }' >foo.c
echo 'int two () { return 2; }' >bar.c

mkdir empty

libtoolize
$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

cwd=$(pwd) || fatal_ "getting current working directory"

# Install libraries in lib/, and the rest in empty/.
# (in fact there is no "rest", so as the name imply empty/ is
# expected to remain empty).
./configure --prefix="$cwd/empty" --libdir="$cwd/lib"

$MAKE
test -f lib1foo.la
test -f lib1bar.la
test -f lib2foo.la
test ! -e lib2bar.la
test ! -e lib3foo.la
test ! -e lib3bar.la

$MAKE check
test ! -e lib2bar.la
test -f lib3foo.la
test ! -e lib3bar.la

$MAKE install
test -f lib/lib1foo.la
test -f lib/lib1bar.la
test -f lib/lib2foo.la
test ! -e lib/lib3foo.la
find empty -type f -print > empty.lst
test -s empty.lst && { cat empty.lst; exit 1; }

$MAKE uninstall
find lib -type f -print > lib.lst
test -s lib.lst && { cat lib.lst; exit 1; }
test -f lib1foo.la
test -f lib1bar.la
test -f lib2foo.la
test -f lib3foo.la

$MAKE clean
test ! -e lib1foo.la
test ! -e lib1bar.la
test ! -e lib2foo.la
test ! -e lib3foo.la

:
