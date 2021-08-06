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
#  - a "Bail out!" directive causes an hard error, even if coming after
#    a "SKIP plan" (e.g., "1..0 # SKIP").

. test-init.sh

. tap-setup.sh

cat > all.test <<END
1..0 # SKIP
Bail out!
END

run_make -O -e FAIL check
count_test_results total=2 pass=0 fail=0 xpass=0 xfail=0 skip=1 error=1
grep '^ERROR: all\.test - Bail out!' stdout

:
