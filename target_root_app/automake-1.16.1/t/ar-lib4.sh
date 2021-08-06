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

# Test if lib_LTLIBRARIES requests AM_PROG_AR.

required=libtoolize
. test-init.sh

cp configure.ac X

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_LIBTOOL
AC_OUTPUT
END

cat > Makefile.am << 'END'
lib_LTLIBRARIES = libfoo.la
libfoo_la_SOURCES = foo.c
END

libtoolize
$ACLOCAL
AUTOMAKE_fails

grep 'requires.*AM_PROG_AR' stderr

rm -rf autom4te*.cache

cp X configure.ac

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_PROG_LIBTOOL
AC_OUTPUT
END

$ACLOCAL
$AUTOMAKE --add-missing

:
