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
#  - literal "0" and "0.0" in a test description and a TODO/SKIP message
#    at the same time

. test-init.sh

. tap-setup.sh

cat > all.test << 'END'
1..14

ok 1 0
ok 2 0.0

ok 3 0   # TODO 0
ok 4 0.0 # TODO 0
ok 5 0   # TODO 0.0
ok 6 0.0 # TODO 0.0

not ok 7 0   # TODO 0
not ok 8 0.0 # TODO 0
not ok 9 0   # TODO 0.0
not ok 10 0.0 # TODO 0.0

ok 11 0   # SKIP 0
ok 12 0.0 # SKIP 0
ok 13 0   # SKIP 0.0
ok 14 0.0 # SKIP 0.0

END

run_make -O -e FAIL check
count_test_results total=14 pass=2 fail=0 xpass=4 xfail=4 skip=4 error=0

sed '/^ *$/d' > exp << 'END'

PASS: all.test 1 0
PASS: all.test 2 0.0

XPASS: all.test 3 0 # TODO 0
XPASS: all.test 4 0.0 # TODO 0
XPASS: all.test 5 0 # TODO 0.0
XPASS: all.test 6 0.0 # TODO 0.0

XFAIL: all.test 7 0 # TODO 0
XFAIL: all.test 8 0.0 # TODO 0
XFAIL: all.test 9 0 # TODO 0.0
XFAIL: all.test 10 0.0 # TODO 0.0

SKIP: all.test 11 0 # SKIP 0
SKIP: all.test 12 0.0 # SKIP 0
SKIP: all.test 13 0 # SKIP 0.0
SKIP: all.test 14 0.0 # SKIP 0.0

END

$FGREP ': all.test' stdout > got

cat exp
cat got
diff exp got

:
