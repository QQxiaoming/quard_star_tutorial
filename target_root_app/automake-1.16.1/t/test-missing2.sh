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
#  - non-existent scripts listed in TESTS get diagnosed, even when
#    all the $(TEST_LOGS) have a dummy dependency.
# See also related test 'test-missing.sh'.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = foobar1.test foobar2.test
$(TEST_LOGS):
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

$MAKE foobar1.log foobar2.log || exit 99
test ! -e foobar1.log || exit 99
test ! -e foobar1.trs || exit 99
test ! -e foobar2.log || exit 99
test ! -e foobar2.trs || exit 99

run_make -e FAIL -M check
grep 'test-suite\.log.*foobar1\.log' output
grep 'test-suite\.log.*foobar1\.trs' output
grep 'test-suite\.log.*foobar2\.log' output
grep 'test-suite\.log.*foobar2\.trs' output
test ! -e test-suite.log

:
