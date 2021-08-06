#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Make sure libtool clean targets exist.
# Report from Eric Magnien.

required=libtoolize
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_LIBTOOL
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
SUBDIR = subdir
lib_LTLIBRARIES = libfoo.la
libfoo_la_SOURCES = foo.c
END

mkdir sub
cat > sub/Makefile.am << 'END'
lib_LTLIBRARIES = libfoo.la
libfoo_la_SOURCES = foo.c
END

$ACLOCAL
: > ltmain.sh
$AUTOMAKE -a

grep 'rm -f .*\.lo' sub/Makefile.in

:
