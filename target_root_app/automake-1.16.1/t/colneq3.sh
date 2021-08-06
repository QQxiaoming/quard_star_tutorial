#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Test that := definitions work as expected at make time.

required=GNUmake
. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
BAR := $(FOO)
BAZ = $(FOO)
FOO := foo
.PHONY: test
test:
	test x'$(FOO)' = x'foo'
	test x'$(BAZ)' = x'foo'
	test x'$(BAR)' = x
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -Wno-portability

./configure
$MAKE test

:
