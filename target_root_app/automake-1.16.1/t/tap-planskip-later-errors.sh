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
#  - TAP errors following a "TAP plan with SKIP" are still diagnosed.

. test-init.sh

. tap-setup.sh

cat > all.test <<END
1..0
a non-TAP line
1..3
1..5
# a comment
1..1
END

run_make -O -e FAIL check
count_test_results total=4 pass=0 fail=0 xpass=0 xfail=0 skip=1 error=3
test $(grep -c '^ERROR: all\.test - multiple test plans' stdout) -eq 3

:
