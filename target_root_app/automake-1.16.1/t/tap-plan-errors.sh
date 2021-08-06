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

# TAP support: the following situations should be flagged as errors:
#  - unmatched test plan (too few tests run)
#  - multiple test plans
#  - missing test plan
#  - misplaced test plan (tests run after a late plan)
# Checks about unplanned tests are performed in 'tap-unplanned.sh'
# and 'tap-skip-whole-badcount.sh'.  More checks about corner-cases
# in TAP plans are performed in 'tap-plan-corner.sh'.

. test-init.sh

. tap-setup.sh

my_check ()
{
  cat > all.test
  test -n "$err" || fatal_ "\$err not set before calling my_check"
  cat all.test # For debugging.
  run_make -O -e FAIL check
  count_test_results "$@"
  grep "^ERROR: all\\.test $err$" stdout
  unset err
}

err='- too few tests run (expected 2, got 1)'
my_check total=2 pass=1 fail=0 xpass=0 xfail=0 skip=0 error=1 <<END
1..2
ok 1
END

err='- too few tests run (expected 12, got 3)'
my_check total=4 pass=2 fail=0 xpass=0 xfail=1 skip=0 error=1 <<END
ok 1
ok 2
not ok 3 # TODO
1..12
END

err='- too few tests run (expected 1, got 0)'
my_check total=1 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=1 <<END
1..1
END

err='2 # AFTER LATE PLAN'
my_check total=2 pass=1 fail=0 xpass=0 xfail=0 skip=0 error=1 <<END
ok 1
1..2
ok 2
END

err='5 # AFTER LATE PLAN'
my_check total=5 pass=4 fail=0 xpass=0 xfail=0 skip=0 error=1 <<END
ok 1
ok 2
ok 3
ok 4
1..5
ok 5
END

err='- missing test plan'
my_check total=2 pass=1 fail=0 xpass=0 xfail=0 skip=0 error=1 <<END
ok 1
END


# The two test plans here are deliberately equal.
err='- multiple test plans'
my_check total=3 pass=2 fail=0 xpass=0 xfail=0 skip=0 error=1 <<END
1..2
ok 1
ok 2
1..2
END

# The second plan is diagnosed as extra, and only the first one is
# relevant w.r.t. the number of the expected test.
err='- multiple test plans'
my_check total=4 pass=3 fail=0 xpass=0 xfail=0 skip=0 error=1 <<END
1..3
ok 1
ok 2
1..2
ok 3
END

# As above, in a slightly different situation.
err='- multiple test plans'
my_check total=3 pass=2 fail=0 xpass=0 xfail=0 skip=0 error=1 <<END
1..2
ok 1
ok 2
1..4
END

:
