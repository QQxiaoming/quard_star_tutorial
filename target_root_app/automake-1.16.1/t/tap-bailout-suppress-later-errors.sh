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
#  - A "Bail out!" directive causes the driver to ignore any TAP
#    result or error in the rest of the following TAP stream.

. test-init.sh

. tap-setup.sh

# Various errors that can all be squashed into a single test script.
cat > foo.test << 'END'
1..5
Bail out!
# All possible test results.
# Test out-of-order.
ok 4
# Extra TAP plan.
1..2
# Another bailout directive.
Bail out! Not seen.
# Stop now, with too few tests run.
END

# Tests run after a "SKIP" plan.
cat > bar.test << 'END'
1..0 # SKIP
Bail out!
ok 1
END

# Too many tests run.
cat > baz.test << 'END'
1..1
ok 1
Bail out!
ok 2
ok 3
END

run_make -e FAIL -O TESTS='foo.test bar.test baz.test' check
count_test_results total=5 pass=1 fail=0 xpass=0 xfail=0 skip=1 error=3

grep '^ERROR: foo\.test - Bail out!$' stdout
grep '^ERROR: bar\.test - Bail out!$' stdout
grep '^SKIP: bar\.test'               stdout
grep '^ERROR: baz\.test - Bail out!$' stdout
grep '^PASS: baz\.test 1$'            stdout

$FGREP 'Not seen' stdout && exit 1

test $($FGREP -c ': foo.test' stdout) -eq 1
test $($FGREP -c ': bar.test' stdout) -eq 2
test $($FGREP -c ': baz.test' stdout) -eq 2

:
