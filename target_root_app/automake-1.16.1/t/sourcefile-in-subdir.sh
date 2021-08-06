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

# Test to make sure subdir source file generates explicit dependency.

. test-init.sh

cat > Makefile.am << 'END'
bin_PROGRAMS = zardoz widdershins
zardoz_SOURCES = y.c x/z.c
widdershins_SOURCES = x/z.c
END

cat >> configure.ac << 'END'
AC_PROG_CC
END

$ACLOCAL
$AUTOMAKE -Wno-unsupported

grep '^z\.o: x/z\.c$' Makefile.in

:
