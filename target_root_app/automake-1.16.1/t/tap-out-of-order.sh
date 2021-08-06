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
#  - out-of-order test results

. test-init.sh

. tap-setup.sh

cat > a.test <<END
1..3
ok
not ok 2 # TODO
ok 71
END

cat > b.test <<END
1..5
ok 1
ok 2
ok 4 foo # SKIP mu
not ok 2 bar # TODO um
ok 5
END

cat > c.test <<END
1..4
ok 1
ok foo
ok 4 - zardoz
not ok 3
END

# This is documented as an "test out of sequence" error in the
# TAP::Parser documentation.  Keep it in sync with the example
# there.
cat > d.test <<'END'
1..5
ok 1
ok 2
ok 15
ok 16
ok 17
END

run_make -O -e FAIL TESTS='a.test b.test c.test d.test' check
count_test_results total=17 pass=8 fail=0 xpass=0 xfail=1 skip=0 error=8

test $($FGREP -c ': a.test' stdout) -eq 3
test $($FGREP -c ': b.test' stdout) -eq 5
test $($FGREP -c ': c.test' stdout) -eq 4
test $($FGREP -c ': d.test' stdout) -eq 5

grep '^ERROR: a\.test 71 # OUT-OF-ORDER (expecting 3)$' stdout
grep '^ERROR: b\.test 4 foo # OUT-OF-ORDER (expecting 3)$' stdout
grep '^ERROR: b\.test 2 bar # OUT-OF-ORDER (expecting 4)$' stdout
grep '^ERROR: c\.test 4 - zardoz # OUT-OF-ORDER (expecting 3)$' stdout
grep '^ERROR: c\.test 3 # OUT-OF-ORDER (expecting 4)$' stdout
grep '^ERROR: d\.test 15 # OUT-OF-ORDER (expecting 3)$' stdout
grep '^ERROR: d\.test 16 # OUT-OF-ORDER (expecting 4)$' stdout
grep '^ERROR: d\.test 17 # OUT-OF-ORDER (expecting 5)$' stdout

:
