#! /bin/sh
# Copyright (C) 1997-2018 Free Software Foundation, Inc.
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

# Test to make sure several config headers are allowed.
# See also sister "semantic" test 'confh8.sh'.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_HEADERS([config.h two.h])
END

: > Makefile.am

: > config.h.in
: > two.h.in

$ACLOCAL
$AUTOMAKE

# Try again with more macros.

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

$ACLOCAL --force
$AUTOMAKE

:
