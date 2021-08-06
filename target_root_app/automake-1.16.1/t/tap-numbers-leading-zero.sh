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
#  - how does TAP result numbers with leading zero fares?

. test-init.sh

. tap-setup.sh

do_checks ()
{
  run_make -O -e FAIL check
  count_test_results "$@"
  # Allow some normalization of leading zeros, without forcing it.
  LC_ALL=C sed -n 's/: all\.test 0*\([0-9]\)/: all.test \1/p' stdout > got
  cat exp
  cat got
  diff exp got
}

cat > all.test <<END
1..15

ok 01
ok 0002

not ok 03
not ok 0004

ok 05 # SKIP
ok 0006 # SKIP

not ok 07 # TODO
not ok 0008 # TODO

ok 009
ok 010

ok 00000011
not ok 0012
ok 00000013 # SKIP
not ok 0014 # TODO
ok 00000015 # TODO
END

cat > exp <<END
PASS: all.test 1
PASS: all.test 2
FAIL: all.test 3
FAIL: all.test 4
SKIP: all.test 5 # SKIP
SKIP: all.test 6 # SKIP
XFAIL: all.test 7 # TODO
XFAIL: all.test 8 # TODO
PASS: all.test 9
PASS: all.test 10
PASS: all.test 11
FAIL: all.test 12
SKIP: all.test 13 # SKIP
XFAIL: all.test 14 # TODO
XPASS: all.test 15 # TODO
END

do_checks total=15 pass=5 fail=3 xpass=1 xfail=3 skip=3 error=0

cat > all.test <<END
1..8

ok 010
not ok 003
ok 0001 # SKIP
not ok 010 # TODO
ok 00100 # TODO

ok 06
ok 00007
ok 8
END

cat > exp <<END
ERROR: all.test 10 # OUT-OF-ORDER (expecting 1)
ERROR: all.test 3 # OUT-OF-ORDER (expecting 2)
ERROR: all.test 1 # OUT-OF-ORDER (expecting 3)
ERROR: all.test 10 # OUT-OF-ORDER (expecting 4)
ERROR: all.test 100 # OUT-OF-ORDER (expecting 5)
PASS: all.test 6
PASS: all.test 7
PASS: all.test 8
END

do_checks total=8 pass=3 fail=0 xpass=0 xfail=0 skip=0 error=5

:
