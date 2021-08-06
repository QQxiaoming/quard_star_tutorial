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

# TAP support: a malformed TAP plan is not recognized.  The checks in
# here should be consistent with the behaviour of the 'prove' utility.

. test-init.sh

. tap-setup.sh

cat > a.test <<END
1..1 foo
END

cat > b.test <<END
 1..2
ok 1
ok 2
END

cat > c.test <<END
1..1 # SKIP
END

cat > d.test <<END
1..2 # foo bar
not ok 1 # TODO
not ok 2 # TODO
END

cat > e.test <<END
0..0
END

tests_list=$(echo *.test)

run_make -O -e FAIL TESTS="$tests_list" check
count_test_results total=9 pass=2 fail=0 xpass=0 xfail=2 skip=0 error=5

for tst in $tests_list; do
  grep "^ERROR: $tst - missing test plan$" stdout
done

:
