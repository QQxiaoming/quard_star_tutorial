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

# Test to make sure no-dependencies option does the right thing.
# Bug report from Greg A. Woods.

. test-init.sh

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = no-dependencies
bin_PROGRAMS = zardoz
zardoz_SOURCES = y.c
END

cat >> configure.ac << 'END'
AC_PROG_CC
END

mkdir x

: > y.c

$ACLOCAL
$AUTOMAKE

sed 's/printf .*%s//' Makefile.in > Makefile.tmp
grep '%' Makefile.tmp && exit 1

:
