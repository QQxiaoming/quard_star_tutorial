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
#  - a "not ok # SKIP" line should count as a failure, for consistency
#    with the prove(1) utility.

. test-init.sh

. tap-setup.sh

cat > all.test <<'END'
1..4
not ok # SKIP
not ok 2 # SKIP
not ok - foo # SKIP
not ok 4 - bar # SKIP
END

run_make -O -e FAIL check
count_test_results total=4 pass=0 fail=4 skip=0 xpass=0 xfail=0 error=0

:
