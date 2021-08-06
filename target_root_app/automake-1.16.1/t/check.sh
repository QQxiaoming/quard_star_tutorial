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

# Test Automake style tests.

# For gen-testsuite-part: ==> try-with-serial-tests <==
. test-init.sh

cat > Makefile.am << 'END'
TESTS = frob.test
END

test x"$am_serial_tests" = x"yes" || : > test-driver

$ACLOCAL
$AUTOMAKE

grep 'check-TESTS.*:' Makefile.in
grep 'check-DEJAGNU' Makefile.in && exit 1

# 'check-TESTS' is phony.
sed -n '/^\.PHONY:/,/^$/p' Makefile.in | $EGREP '(^| )check-TESTS($| )'

# 'check' should depend directly on 'check-am' (similar tests are
# in check2.sh and check3.sh).
$EGREP '^check:.* check-am( |$)' Makefile.in

:
