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
#  - handling of "ambiguous" TODO and SKIP directives
# See also related test 'tap-todo-skip-together.sh'.

. test-init.sh

. tap-setup.sh

cat > all.test <<END
1..6
ok 1 # foo SKIP
not ok 2 # bar TODO
ok 3 # :SKIP
not ok 4 # :TODO
ok 5 # SKIP SKIP
not ok 6 # TODO TODO
END

run_make -O -e FAIL check
count_test_results total=6 pass=2 fail=2 xpass=0 xfail=1 skip=1 error=0

cat > exp << 'END'
PASS: all.test 1 # foo SKIP
FAIL: all.test 2 # bar TODO
PASS: all.test 3 # :SKIP
FAIL: all.test 4 # :TODO
SKIP: all.test 5 # SKIP SKIP
XFAIL: all.test 6 # TODO TODO
END

$FGREP ': all.test' stdout > got

cat exp
cat got
diff exp got

:
