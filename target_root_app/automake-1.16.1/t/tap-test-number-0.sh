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
#  - a test result numbered as 0 is to be considered out-of-order
# This is consistent with the behaviour of the 'prove' utility.

. test-init.sh
. tap-setup.sh

cat > a.test <<END
1..1
ok 0
END

cat > b.test <<END
1..1
not ok 0
END

cat > c.test <<END
1..1
ok 0 foo # SKIP
END

cat > d.test <<END
1..1
not ok 0 bar # TODO
END

cat > e.test <<END
1..1
ok 0 # TODO
END

run_make -O -e FAIL TESTS='a.test b.test c.test d.test e.test' check
count_test_results total=5 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=5

grep '^ERROR: a\.test 0 # OUT-OF-ORDER (expecting 1)$' stdout
grep '^ERROR: b\.test 0 # OUT-OF-ORDER (expecting 1)$' stdout
grep '^ERROR: c\.test 0 foo # OUT-OF-ORDER (expecting 1)$' stdout
grep '^ERROR: d\.test 0 bar # OUT-OF-ORDER (expecting 1)$' stdout
grep '^ERROR: e\.test 0 # OUT-OF-ORDER (expecting 1)$' stdout

:
