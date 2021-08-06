#! /bin/sh
# Copyright (C) 2007-2018 Free Software Foundation, Inc.
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

# Make sure .libs directories are removed for _PROGRAMS.
# Report from Guillermo Ontañón.

required='cc libtoolize'
. test-init.sh

cat >> configure.ac << 'END'
AM_PROG_AR
AC_PROG_LIBTOOL
AC_OUTPUT
END

cat > Makefile.am << 'END'
lib_LTLIBRARIES = lib/libfoo.la
lib_libfoo_la_SOURCES = foo.c
bin_PROGRAMS = src/main
check_PROGRAMS = check/test
src_main_SOURCES = main.c
check_test_SOURCES = main.c
LDADD = lib/libfoo.la
END

mkdir lib src check
cat > foo.c << 'END'
int foo () { return 0; }
END

cat > main.c << 'END'
extern int foo ();
int main () { return foo (); }
END

libtoolize
$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF
./configure
$MAKE all check
$MAKE clean
test ! -e src/.libs
test ! -e src/_libs
test ! -e check/.libs
test ! -e check/_libs
$MAKE distcheck

:
