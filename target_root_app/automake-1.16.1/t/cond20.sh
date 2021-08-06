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

# Regression test for recursion handling in substitution references to
# conditional variables.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_CONDITIONAL([COND1], [true])
END

cat > Makefile.am << 'END'
var1 = $(var2)

if COND1
var2 = $(var1:=.c) foo.c
else
var2 = $(var1:=.c)
endif

helldl_SOURCES = $(var2)

bin_PROGRAMS = helldl
END

$ACLOCAL
$AUTOCONF
AUTOMAKE_fails -a
grep "variable.*var2.*recursively defined" stderr

:
