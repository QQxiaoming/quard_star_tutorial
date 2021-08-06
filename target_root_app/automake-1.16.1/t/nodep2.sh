#! /bin/sh
# Copyright (C) 2006-2018 Free Software Foundation, Inc.
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

# Another test to make sure no-dependencies option does the right thing.

. test-init.sh

cat > Makefile.am << 'END'
bin_PROGRAMS = foo
foo_SOURCES = a.c b.cpp c.m cxx.mm d.S e.java f.upc
END

cat > configure.ac << 'END'
AC_INIT([nodep2], [1], [bug-automake@gnu.org])
AM_INIT_AUTOMAKE([no-dependencies])
AC_CONFIG_FILES([Makefile])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_OBJC
AC_PROG_OBJCXX
AM_PROG_AS
AM_PROG_GCJ
AM_PROG_UPC
AC_OUTPUT
END

$ACLOCAL
$AUTOMAKE

grep DEPMODE Makefile.in && exit 1

:
