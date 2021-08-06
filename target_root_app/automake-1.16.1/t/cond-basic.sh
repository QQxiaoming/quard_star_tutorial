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

# Check basic use of conditionals.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([TEST], [true])
AC_OUTPUT
END

cat > Makefile.am << 'END'
if TEST
target: true
	action
else
target: false
endif
END

$ACLOCAL
$AUTOMAKE

grep '^@TEST_TRUE@target: true' Makefile.in
grep "^@TEST_TRUE@${tab}action" Makefile.in
grep '^@TEST_FALSE@target: false' Makefile.in

:
