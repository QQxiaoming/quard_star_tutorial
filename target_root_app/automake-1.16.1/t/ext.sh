#! /bin/sh
# Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

# Test to make sure extensions are set correctly for various languages.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_F77
AC_PROG_FC
AC_PROG_OBJC
AC_PROG_OBJCXX
AM_PROG_UPC
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo
foo_SOURCES = 1.f 2.for 3.f90 4.f95 5.F 6.F90 7.F95 8.r 9.m 10.mm 11.upc
END

$ACLOCAL
$AUTOMAKE

for ext in f for f90 f95 F F90 F95 r m mm upc; do
   grep "^\.$ext\.o:" Makefile.in
   grep "^$ext\.o:" Makefile.in && exit 1
   : For shells with busted 'set -e'.
done

:
