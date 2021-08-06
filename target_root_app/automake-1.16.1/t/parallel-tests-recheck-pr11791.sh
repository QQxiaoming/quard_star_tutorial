#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# parallel-tests: "make recheck" and "make -k recheck" in the face of
# build failures for the test cases.  See automake bug#11791.

required='cc native'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = $(EXTRA_PROGRAMS)
EXTRA_PROGRAMS = foo
END

echo 'int main (void) { return 1; }' > foo.c

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

run_make -O -e FAIL check
count_test_results total=1 pass=0 fail=1 xpass=0 xfail=0 skip=0 error=0

using_gmake || $sleep # Required by BSD make.
run_make -O -e IGNORE -- -k recheck
# Don't trust the exit status of "make -k" for non-GNU makes.
! using_gmake || test $am_make_rc -gt 0 || exit 1
count_test_results total=1 pass=0 fail=1 xpass=0 xfail=0 skip=0 error=0

# Introduce an error in foo.c, that should cause a compilation failure.
$sleep
echo choke me >> foo.c

run_make -O -e FAIL recheck
# We don't get a change to run the testsuite.
$EGREP '(X?PASS|X?FAIL|SKIP|ERROR):' stdout && exit 1
# These shouldn't be removed, otherwise the next make recheck will do
# nothing.
test -f foo.log
test -f foo.trs

using_gmake || $sleep # Required by BSD make.
run_make -O -e IGNORE -- -k recheck
# Don't trust the exit status of "make -k" for non-GNU makes.
! using_gmake || test $am_make_rc -gt 0 || exit 1
# We don't get a change to run the testsuite.
$EGREP '(X?PASS|X?FAIL|SKIP|ERROR):' stdout && exit 1
test -f foo.log
test -f foo.trs

# "Repair" foo.c, and expect everything to work.
$sleep
echo 'int main (void) { return 0; }' > foo.c

run_make -O recheck
count_test_results total=1 pass=1 fail=0 xpass=0 xfail=0 skip=0 error=0
test -f foo.log
test -f foo.trs

run_make -O recheck
count_test_results total=0 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=0
test -f foo.log
test -f foo.trs

:
