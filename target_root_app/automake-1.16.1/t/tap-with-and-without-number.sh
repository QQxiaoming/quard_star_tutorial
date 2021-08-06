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
#  - numbered and unnumbered test results can coexist
#  - tests without explicit number get automatically numbered in the
#    testsuite progress output on console

. test-init.sh

. tap-setup.sh

cat > all.test <<'END'
1..7
ok 1 foo
ok 2 - foo2
ok - bar
not ok baz # TODO
not ok 5 - asd # TODO
ok 6 # SKIP
ok zardoz
END

run_make -O check
count_test_results total=7 pass=4 fail=0 xpass=0 xfail=2 skip=1 error=0

grep '^PASS: all\.test 1 foo$' stdout
grep '^PASS: all\.test 2 - foo2$' stdout
grep '^PASS: all\.test 3 - bar$' stdout
grep '^XFAIL: all\.test 4 baz # TODO$' stdout
grep '^XFAIL: all\.test 5 - asd # TODO$' stdout
grep '^SKIP: all\.test 6 # SKIP$' stdout
grep '^PASS: all\.test 7 zardoz$' stdout

:
