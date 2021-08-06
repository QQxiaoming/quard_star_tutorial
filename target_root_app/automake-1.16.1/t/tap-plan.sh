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
#  - test scripts with the test plan at the beginning
#  - test scripts with the test plan at the end

. test-init.sh

. tap-setup.sh

cat > top.test <<END
1..3
ok 1
ok 2 # SKIP
ok 3
# a trailing comment
END

cat > bot.test <<END
ok 1
# a comment
ok 2
not ok 3 # TODO
# another comment
ok
1..4
END

for tap_flags in "" "--comments"; do
  run_make -O check \
    TESTS='top.test bot.test' \
    TEST_LOG_DRIVER_FLAGS="$tap_flags"
  count_test_results total=7 pass=5 xfail=1 skip=1 fail=0 xpass=0 error=0
done


:
