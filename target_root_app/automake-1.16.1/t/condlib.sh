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

# Test for bug with conditional library.
# From Harlan Stenn.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_RANLIB
AM_MAINTAINER_MODE
AM_PROG_AR
AC_PROG_CC
END

cat > Makefile.am << 'END'
noinst_LIBRARIES = @LIBRSAREF@
EXTRA_LIBRARIES = librsaref.a

nodist_librsaref_a_SOURCES = desc.c digit.c

BUILT_SOURCES = $(nodist_librsaref_a_SOURCES)
END

: > ar-lib

$ACLOCAL
$AUTOMAKE

$FGREP librsaref.a.c Makefile.in && exit 1
exit 0
