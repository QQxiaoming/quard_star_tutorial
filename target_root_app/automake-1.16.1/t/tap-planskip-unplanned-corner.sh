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
#  - test results seen in a TAP stream that has a "plan with SKIP" are
#    flagged as errors, even if all of them are "SKIP" results.

. test-init.sh

. tap-setup.sh

cat > foo.test <<END
1..0 # SKIP
ok 1 # SKIP
END

cat > bar.test <<END
ok 1 # SKIP
1..0 # SKIP
END

run_make -O -e FAIL TESTS='foo.test bar.test' check
count_test_results total=5 pass=0 fail=0 xpass=0 xfail=0 skip=2 error=3

# Don't be too strict w.r.t. the format of the "skip whole test" message.
grep '^SKIP: foo\.test$' stdout || grep '^SKIP: foo\.test [^0-9]' stdout
grep '^ERROR: foo\.test 1 # UNPLANNED$' stdout
grep '^ERROR: foo\.test - too many tests run (expected 0, got 1)$' stdout
grep '^SKIP: bar\.test 1 # SKIP$' stdout
grep '^ERROR: bar\.test - too many tests run (expected 0, got 1)$' stdout

:
