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

# Superficial test to check that dejagnu tests and automake-style
# tests can coexist.  See also related deeper test 'check12.sh'.

. test-init.sh

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = dejagnu
TESTS = frob.test
END

test x"$am_serial_tests" = x"yes" || : > test-driver

$ACLOCAL
$AUTOMAKE

grep '^check-TESTS' Makefile.in
grep '^check-DEJAGNU' Makefile.in

:
