#! /bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

# This tests for a bug reported by Henrik Frystyk Nielsen <frystyk@w3.org>
# In some cases the auto-dependency can get confused and try
# to '-include' a directory (if a backslash-newline appears in _SOURCES).

. test-init.sh

echo AC_PROG_CC >> configure.ac

cat > Makefile.am << 'END'
bin_PROGRAMS = fred
fred_SOURCES = \
   fred.c
END

$ACLOCAL
$AUTOMAKE

test 1 -eq $(grep -c '^@AMDEP_TRUE@@am__include@' Makefile.in)

:
