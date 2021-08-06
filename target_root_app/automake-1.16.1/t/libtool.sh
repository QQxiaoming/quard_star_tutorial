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

# Make sure libtool is removed.
# Report from Kevin Dalley.

required=libtool
. test-init.sh

cat >> configure.ac << 'END'
AC_LIBTOOL_DLOPEN
AC_DISABLE_SHARED
AC_PROG_LIBTOOL
AC_SUBST([LIBTOOL_DEPS])
END

: > Makefile.am

: > ltmain.sh
: > config.guess
: > config.sub

$ACLOCAL
$AUTOMAKE

grep 'rm -f libtool' Makefile.in

:
