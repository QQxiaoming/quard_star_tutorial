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
#  - unnumbered tests are OK, as long as their final total number
#    agrees with the plan
#  - test results without number get automatically numbered in the
#    console progress output

. test-init.sh

. tap-setup.sh

cat > all.test <<'END'
1..14
ok
ok foo
ok - foo2
not ok
not ok bar
not ok - bar2
; See that we can intermingle different kind of results without
; messing up the autonumbering
ok
ok # TODO
not ok # TODO who cares?
ok
not ok
ok muuu # SKIP
not ok
ok
END

run_make -O -e FAIL TESTS=all.test check
count_test_results total=14 pass=6 fail=5 xpass=1 xfail=1 skip=1 error=0

cat > exp <<'END'
PASS: all.test 1
PASS: all.test 2 foo
PASS: all.test 3 - foo2
FAIL: all.test 4
FAIL: all.test 5 bar
FAIL: all.test 6 - bar2
PASS: all.test 7
XPASS: all.test 8 # TODO
XFAIL: all.test 9 # TODO who cares?
PASS: all.test 10
FAIL: all.test 11
SKIP: all.test 12 muuu # SKIP
FAIL: all.test 13
PASS: all.test 14
END

$FGREP ': all.test' stdout > got

cat exp
cat got
diff exp got

:
