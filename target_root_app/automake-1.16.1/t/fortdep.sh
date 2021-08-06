#! /bin/sh
# Copyright (C) 2000-2018 Free Software Foundation, Inc.
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

# Test to make sure dependency tracking doesn't interfere with Fortran.
# For PR 75.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_SRCDIR([foo.f])
AC_PROG_F77
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS=foo
foo_SOURCES=foo.f
END

: > foo.f

$ACLOCAL
$AUTOMAKE

grep 'foo\.Po' Makefile.in && exit 1
exit 0
