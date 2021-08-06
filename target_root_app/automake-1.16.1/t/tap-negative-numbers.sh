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
#  - don't spuriously recognize negative TAP result numbers, but correctly
#    interpret them as test descriptions instead

. test-init.sh

. tap-setup.sh

cat > all.test <<'END'
1..7
ok -1
not ok -3
ok -2 # SKIP
not ok -5 # TODO
ok -04 # TODO
ok -121
not ok -50000
END

run_make -O -e FAIL check
count_test_results total=7 pass=2 fail=2 xpass=1 xfail=1 skip=1 error=0

cat > exp <<'END'
PASS: all.test 1 -1
FAIL: all.test 2 -3
SKIP: all.test 3 -2 # SKIP
XFAIL: all.test 4 -5 # TODO
XPASS: all.test 5 -04 # TODO
PASS: all.test 6 -121
FAIL: all.test 7 -50000
END

$FGREP ': all.test' stdout > got

cat exp
cat got
diff exp got

:
