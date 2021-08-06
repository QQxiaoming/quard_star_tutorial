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

# TAP support: some unusual forms for valid TAP input.
# See also related test 'tap-fancy2.sh'.

. test-init.sh

. tap-setup.sh

#
# From manpage Test::Harness::TAP(3):
#
#   Lines written to standard output matching /^(not )?ok\b/ must be
#   interpreted as test lines. All other lines must not be considered
#   test output.
#
# Unfortunately, the exact format of TODO and SKIP directives is not as
# clearly described in that manpage; but a simple reverse-engineering of
# the prove(1) utility shows that it is probably given by the perl regex
# /#\s*(TODO|SKIP)\b/.
#

cat > all.test <<END
1..21

ok? a question
not ok? a question

ok+plus
not ok+plus

ok-minus
not ok-minus

ok#55
not ok#55

ok${tab}  ${tab}9
ok ${tab}${tab}          10

not ok${tab}  ${tab}11
not ok ${tab}${tab}          12

ok# SKIP
ok${tab}#SKIP--who cares?
ok?#SKIP!
ok!#SKIP?

not ok# TODO
not ok${tab}#TODO--who cares?
not ok?#TODO!
not ok!#TODO?

ok~#TODO
END

run_make -O -e FAIL check
count_test_results total=21 pass=6 fail=6 xfail=4 xpass=1 skip=4 error=0

#
# "Weird" characters support.
#

# The "#" character might cause confusion w.r.t. TAP directives (TODO,
# SKIP), so we don't attempt to use it.
weirdchars=\''"$!&()[]<>;^?*/@%=,.:'

cat > all.test <<END
1..6
ok $weirdchars
not ok $weirdchars
ok $weirdchars # TODO
not ok $weirdchars # TODO
ok $weirdchars # SKIP
Bail out! $weirdchars
END

run_make -O -e FAIL check
count_test_results total=6 pass=1 fail=1 xfail=1 xpass=1 skip=1 error=1
$FGREP "PASS: all.test 1 $weirdchars" stdout
$FGREP "FAIL: all.test 2 $weirdchars" stdout
$FGREP "XPASS: all.test 3 $weirdchars" stdout
$FGREP "XFAIL: all.test 4 $weirdchars" stdout
$FGREP "SKIP: all.test 5 $weirdchars" stdout
$FGREP "ERROR: all.test - Bail out! $weirdchars" stdout

#
# Trailing backslashes does not confuse the parser.
#

bs='\'

cat > all.test <<END
1..6
ok $bs
not ok $bs
ok # TODO $bs
not ok # TODO $bs
ok # SKIP $bs
Bail out! $bs
END

run_make -O -e FAIL check
count_test_results total=6 pass=1 fail=1 xfail=1 xpass=1 skip=1 error=1

grep '^PASS: all\.test 1 \\$' stdout
grep '^FAIL: all\.test 2 \\$' stdout
grep '^XPASS: all\.test 3 # TODO \\$' stdout
grep '^XFAIL: all\.test 4 # TODO \\$' stdout
grep '^SKIP: all\.test 5 # SKIP \\$' stdout
grep '^ERROR: all\.test - Bail out! \\$' stdout

:
