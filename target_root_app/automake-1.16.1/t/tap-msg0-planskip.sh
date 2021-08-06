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
#  - literal strings "0" and "0.0" as the reason of the skip in a "TAP
#    plan with skip" (i.e., "1..0 # SKIP ...").

. test-init.sh

. tap-setup.sh

echo '1..0 # SKIP 0' > a.test
echo '1..0 # SKIP 0.0' > b.test

run_make -O TESTS='a.test b.test' check
count_test_results total=2 pass=0 fail=0 xpass=0 xfail=0 skip=2 error=0

grep '^SKIP: a.test - 0$' stdout
grep '^SKIP: b.test - 0\.0$' stdout

:
