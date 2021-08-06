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

# TAP and $(XFAIL_TESTS): test results without directives are turned from
# PASS to XPASS and from FAIL to XFAIL; other results are unchanged.

. test-init.sh

. tap-setup.sh

echo 'XFAIL_TESTS = $(TESTS)' >> Makefile

cat > all.test <<END
1..6
ok 1
not ok 2
ok 3 # TODO
not ok 4 # TODO
ok 5 # SKIP
# The next should count as a failure (and thus will be
# rendered into an expected failure by XFAIL_TESTS).
not ok 6 # SKIP
Bail out!
END

run_make -O -e FAIL check
count_test_results total=7 pass=0 fail=0 xpass=2 xfail=3 skip=1 error=1

grep '^XPASS: all\.test 1$' stdout
grep '^XFAIL: all\.test 2$' stdout
grep '^XPASS: all\.test 3 # TODO' stdout
grep '^XFAIL: all\.test 4 # TODO' stdout
grep '^SKIP: all\.test 5 # SKIP' stdout
grep '^XFAIL: all\.test 6 # SKIP' stdout
grep '^ERROR: all\.test - Bail out!' stdout

# Check that the exit status of the testsuite is indeed 0 when we
# would expect success.

cat > all.test <<END
1..3
not ok 1
ok 2 # SKIP
not ok 3 # TODO
END

run_make -O check
count_test_results total=3 pass=0 fail=0 xpass=0 xfail=2 skip=1 error=0

:
