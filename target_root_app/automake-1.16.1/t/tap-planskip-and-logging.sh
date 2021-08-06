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
#  - interactions between "TAP plan with SKIP" and logging of earlier or
#    later TAP or non-TAP text

. test-init.sh

. tap-setup.sh

echo TEST_LOG_DRIVER_FLAGS = --comments >> Makefile

cat > foo.test <<END
1..0
a non-TAP line
# a comment
END

cat > foo2.test <<END
a non-TAP line 2
# a comment 2
1..0
END

cat > bar.test <<END
# an early comment
an early non-TAP line
 $tab
1..0 # SKIP
# a later comment
a later non-TAP line
END

run_make -O TESTS='foo.test foo2.test bar.test' check
count_test_results total=3 pass=0 fail=0 xpass=0 xfail=0 skip=3 error=0

grep '^# foo\.test: a comment$' stdout
grep '^# foo2\.test: a comment 2$' stdout
grep '^# bar\.test: an early comment$' stdout
grep '^# bar\.test: a later comment$' stdout

cat foo.log
cat foo2.log
cat bar.log

grep '^a non-TAP line$' foo.log
grep '^# a comment$' foo.log
grep '^a non-TAP line 2$' foo2.log
grep '^# a comment 2$' foo2.log
grep '^# an early comment' bar.log
grep '^an early non-TAP line$' bar.log
grep '^# a later comment' bar.log
grep '^a later non-TAP line$' bar.log
grep "^ $tab$" bar.log

:
