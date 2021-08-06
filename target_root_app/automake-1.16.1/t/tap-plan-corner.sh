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
#  - some corner cases for TAP plan

. test-init.sh

. tap-setup.sh

# -------------------------------------------------------------------------

cat > leading-repeated.test <<END
1..1
1..1
ok 1
END

cat > trailing-repeated.test <<END
ok 1
1..1
1..1
END

for pos in leading trailing; do
  run_make -O -e FAIL TESTS="$pos-repeated.test" check
  count_test_results total=2 pass=1 fail=0 xpass=0 xfail=0 skip=0 error=1
  grep "^ERROR: $pos-repeated\\.test - multiple test plans$" stdout
done

# -------------------------------------------------------------------------

cat > leading-repeated.test <<END
1..2
ok 1
1..2
ok 2
END

cat > trailing-repeated.test <<END
ok 1
1..2
ok 2
1..2
END

run_make -O -e FAIL check \
  TESTS='leading-repeated.test trailing-repeated.test'
count_test_results total=6 pass=3 fail=0 xpass=0 xfail=0 skip=0 error=3
grep "^ERROR: leading-repeated\\.test - multiple test plans$" stdout
grep "^ERROR: trailing-repeated\\.test - multiple test plans$" stdout
grep "^ERROR: trailing-repeated\\.test 2 # AFTER LATE PLAN$" stdout
grep "leading .*AFTER LATE PLAN" stdout && exit 1

# -------------------------------------------------------------------------

cat > 1.test <<END
1..0
1..0
END

cat > 2.test <<END
1..0 # SKIP
1..0
END

cat > 3.test <<END
1..0
1..0 # SKIP
END

cat > 4.test <<END
1..0 # SKIP
1..0 # SKIP
END

run_make -O -e FAIL check TESTS='1.test 2.test 3.test 4.test'
count_test_results total=8 pass=0 fail=0 xpass=0 xfail=0 skip=4 error=4
for i in 1 2 3 4; do
  grep "^ERROR: $i\\.test - multiple test plans$" stdout
done

# -------------------------------------------------------------------------

cat > all.test <<END
1..5
ok 1
ok 2
1..5
ok 3
1..5
ok 4
1..5
ok 5
END

run_make -O -e FAIL check
count_test_results total=8 pass=5 fail=0 xpass=0 xfail=0 skip=0 error=3

cat > exp <<'END'
PASS: all.test 1
PASS: all.test 2
ERROR: all.test - multiple test plans
PASS: all.test 3
ERROR: all.test - multiple test plans
PASS: all.test 4
ERROR: all.test - multiple test plans
PASS: all.test 5
END

$FGREP ': all.test' stdout > got

cat exp
cat got
diff exp got

# -------------------------------------------------------------------------

cat > all.test <<END
1..2
ok 1
ok 2
1..3
ok 3
END

run_make -O -e FAIL check
count_test_results total=5 pass=2 fail=0 xpass=0 xfail=0 skip=0 error=3

cat > exp <<'END'
PASS: all.test 1
PASS: all.test 2
ERROR: all.test - multiple test plans
ERROR: all.test 3 # UNPLANNED
ERROR: all.test - too many tests run (expected 2, got 3)
END

$FGREP ': all.test' stdout > got

cat exp
cat got
diff exp got

# -------------------------------------------------------------------------

cat > all.test <<END
ok 1
ok 2
ok 3
1..5
1..5
ok 4
1..5
ok 5
END

run_make -O -e FAIL check
count_test_results total=7 pass=3 fail=0 xpass=0 xfail=0 skip=0 error=4

cat > exp <<'END'
PASS: all.test 1
PASS: all.test 2
PASS: all.test 3
ERROR: all.test - multiple test plans
ERROR: all.test 4 # AFTER LATE PLAN
ERROR: all.test - multiple test plans
ERROR: all.test 5 # AFTER LATE PLAN
END

$FGREP ': all.test' stdout > got

cat exp
cat got
diff exp got

:
