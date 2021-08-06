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
#  - TODO and SKIP directives on the same line: the first one wins
# See also related test 'tap-ambiguous-directive.sh'.

. test-init.sh

. tap-setup.sh

cat > all.test <<END
1..2
ok 1 # SKIP TODO
not ok 2 # TODO SKIP
END

run_make -O check
count_test_results total=2 pass=0 fail=0 xpass=0 xfail=1 skip=1 error=0

grep '^SKIP: all\.test 1 ' stdout
grep '^XFAIL: all\.test 2 ' stdout

:
