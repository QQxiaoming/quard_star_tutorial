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

# TAP support:
#  - a "plan with skip" given after one or more test result have already
#    been seen is an error
#  - any test result following a "plan with skip" is an error.

. test-init.sh

. tap-setup.sh

cat > all.test <<END
ok 1
1..0 # SKIP too late
END
run_make -O -e FAIL check
count_test_results total=2 pass=1 fail=0 xpass=0 xfail=0 skip=0 error=1
grep '^ERROR: all\.test - too many tests run (expected 0, got 1)$' stdout

cat > all.test <<END
ok 1
ok 2 # SKIP
not ok 3 # TODO
1..0 # SKIP too late
END
run_make -O -e FAIL check
count_test_results total=4 pass=1 fail=0 xpass=0 xfail=1 skip=1 error=1
grep '^ERROR: all\.test - too many tests run (expected 0, got 3)$' stdout

cat > all.test <<END
1..0 # SKIP falsified later
ok 1
END
run_make -O -e FAIL check
count_test_results total=3 pass=0 fail=0 xpass=0 xfail=0 skip=1 error=2
grep '^ERROR: all\.test 1 # UNPLANNED$' stdout
grep '^ERROR: all\.test - too many tests run (expected 0, got 1)$' stdout

cat > all.test <<END
1..0 # SKIP falsified later
ok 1
ok 2 # SKIP
not ok 3
not ok 4 # TODO
END
run_make -O -e FAIL check
count_test_results total=6 pass=0 fail=0 xpass=0 xfail=0 skip=1 error=5
grep '^ERROR: all\.test 1 # UNPLANNED$' stdout
grep '^ERROR: all\.test 2 # UNPLANNED$' stdout
grep '^ERROR: all\.test 3 # UNPLANNED$' stdout
grep '^ERROR: all\.test 4 # UNPLANNED$' stdout
grep '^ERROR: all\.test - too many tests run (expected 0, got 4)$' stdout

:
