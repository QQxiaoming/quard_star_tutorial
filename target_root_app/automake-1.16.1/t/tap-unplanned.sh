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
#  - unplanned tests are properly reported as errors

. test-init.sh

. tap-setup.sh

cat > all.test <<END
1..1
ok 1
ok 2
END
run_make -O -e FAIL check
count_test_results total=3 pass=1 fail=0 xpass=0 xfail=0 skip=0 error=2
grep '^ERROR: all\.test - too many tests run (expected 1, got 2)$' stdout
grep '^ERROR: all\.test 2 # UNPLANNED$' stdout

cat > all.test <<END
1..2
ok 1
ok 2
ok 3
END
run_make -O -e FAIL check
count_test_results total=4 pass=2 fail=0 xpass=0 xfail=0 skip=0 error=2
grep '^ERROR: all\.test - too many tests run (expected 2, got 3)$' stdout
grep '^ERROR: all\.test 3 # UNPLANNED$' stdout

# Interaction with XFAIL_TESTS.
cat > all.test <<END
1..2
not ok 1
ok 2 # SKIP
ok 3
not ok 4
ok 5 # SKIP
END
run_make -O -e FAIL XFAIL_TESTS=all.test check
count_test_results total=6 pass=0 fail=0 xpass=0 xfail=1 skip=1 error=4
grep '^ERROR: all\.test - too many tests run (expected 2, got 5)$' stdout
grep '^ERROR: all\.test 3 # UNPLANNED$' stdout
grep '^ERROR: all\.test 4 # UNPLANNED$' stdout
grep '^ERROR: all\.test 5 # UNPLANNED$' stdout

cat > all.test <<END
1..1
ok 1

ok
ok 3
ok foo
ok 5 - bar bar

not ok
not ok 7
not ok foo
not ok 9 - bar bar

ok # TODO
ok 11 # TODO
ok foo # TODO
ok 13 - bar bar # TODO

not ok # TODO
not ok 15 # TODO
not ok foo # TODO
not ok 17 - bar bar # TODO

ok # SKIP
ok 19 # SKIP
ok foo # SKIP
ok 21 - bar bar # SKIP
END

cat > t <<END

2
3
4 foo
5 - bar bar

6
7
8 foo
9 - bar bar

10
11
12 foo
13 - bar bar

14
15
16 foo
17 - bar bar

18
19
20 foo
21 - bar bar

END

run_make -O -e FAIL check
count_test_results total=22 pass=1 fail=0 xpass=0 xfail=0 skip=0 error=21

echo 'PASS: all.test 1' > exp
sed -e '/^$/d' -e 's/.*/ERROR: all.test & # UNPLANNED/' t >> exp
echo 'ERROR: all.test - too many tests run (expected 1, got 21)' >> exp

$FGREP ': all.test' stdout > got

cat exp
cat got
diff exp got

# Note that, if the TAP input has a trailing plan, it is not possible
# to flag unplanned tests as such, since we do not know they're unplanned
# until the plan is reached; still, we should give at least an error
# message about the unmatched number of tests once we've got the plan.

for x in 'ok' 'ok 3' 'not ok' 'not ok # TODO' 'ok # TODO' 'ok # SKIP'; do
  unindent > all.test <<END
    ok 1
    ok 2 # SKIP
    $x
    1..2
END
  run_make -O -e FAIL check
  test $($FGREP -c ': all.test' stdout) -eq 4
  $EGREP '^PASS: all\.test 1($| )' stdout
  $EGREP '^SKIP: all\.test 2($| )' stdout
  $EGREP ': all\.test 3($| )' stdout
  grep '^ERROR: all\.test - too many tests run (expected 2, got 3)$' stdout
done

:
