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
#  - the string "0" as a test description

. test-init.sh

. tap-setup.sh

cat > all.test << 'END'
1..10
ok 1 0
ok - 0
not ok 3 0
not ok - 0
ok 5 0 # TODO
ok - 0 # TODO
not ok 7 0 # TODO
not ok - 0 # TODO
ok 9 0 # SKIP
ok - 0 # SKIP
END

run_make -O -e FAIL check
count_test_results total=10 pass=2 fail=2 xpass=2 xfail=2 skip=2 error=0

cat > exp << 'END'
PASS: all.test 1 0
PASS: all.test 2 - 0
FAIL: all.test 3 0
FAIL: all.test 4 - 0
XPASS: all.test 5 0 # TODO
XPASS: all.test 6 - 0 # TODO
XFAIL: all.test 7 0 # TODO
XFAIL: all.test 8 - 0 # TODO
SKIP: all.test 9 0 # SKIP
SKIP: all.test 10 - 0 # SKIP
END

$FGREP ': all.test' stdout > got

cat exp
cat got
diff exp got

:
