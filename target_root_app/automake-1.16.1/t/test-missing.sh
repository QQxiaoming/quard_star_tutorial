#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# parallel-tests:
#  - non-existent scripts listed in TESTS get diagnosed
# See also related test 'test-missing2.sh'.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = ok.test zardoz.test
TEST_LOG_COMPILER = true
END

: > ok.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

run_make -e FAIL -M check
test -f ok.log
grep '^PASS: ok\.test' output
$FGREP 'zardoz.log' output
test ! -e test-suite.log

run_make -M -e FAIL TESTS='zardoz2.test' check
$FGREP 'zardoz2.log' output
test ! -e test-suite.log

run_make -M -e FAIL TEST_LOGS='zardoz3.log' check
$FGREP 'zardoz3.log' output
test ! -e test-suite.log

# The errors should persist even after 'test-suite.log'
# has been created.

: > zardoz.test
$MAKE check
rm -f zardoz.test

run_make -M -e FAIL check
$FGREP 'zardoz.log' output
test ! -e test-suite.log

:
