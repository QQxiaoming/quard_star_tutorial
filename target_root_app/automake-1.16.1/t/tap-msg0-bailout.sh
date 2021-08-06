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
#  - literal strings "0" and "0.0" as a test description in the message
#    of a "plan with skip" TAP plan
# generally true!

. test-init.sh

. tap-setup.sh

echo 'Bail out! 0' > a.test
echo 'Bail out! 0.0' > b.test

run_make -O -e FAIL TESTS='a.test b.test' check
count_test_results total=2 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=2

grep '^ERROR: a.test - Bail out! 0$' stdout
grep '^ERROR: b.test - Bail out! 0\.0$' stdout

:
