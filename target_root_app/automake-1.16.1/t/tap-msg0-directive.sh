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
#  - literal string "0" as a TODO or SKIP message

. test-init.sh

. tap-setup.sh

cat > all.test << 'END'
1..3
ok 1 # TODO 0
not ok 2 # TODO 0
ok 3 # SKIP 0
END

run_make -O -e FAIL check
count_test_results total=3 pass=0 fail=0 xpass=1 xfail=1 skip=1 error=0

grep '^XPASS: all\.test 1 # TODO 0$' stdout
grep '^XFAIL: all\.test 2 # TODO 0$' stdout
grep '^SKIP: all\.test 3 # SKIP 0$' stdout

:
