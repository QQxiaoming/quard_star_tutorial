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
#  - TAP plans with numbers having leading zeroes, as in "1..01"
#  - TAP "SKIP" plans with multiple zeroes, as in "1..00 # SKIP"
# This is consistent with the behaviour of the 'prove' utility.

. test-init.sh

. tap-setup.sh

cat > a.test <<END
1..01
ok 1
END

cat > b.test <<END
1..0002
ok 1
ok 2
END

echo 1..010 > c.test
for i in 1 2 3 4 5 6 7 8 9 10; do echo ok $i; done >> c.test

echo 1..00100 > d.test
for i in 1 2 3 4 5 6 7 8 9 10; do
  for j in 1 2 3 4 5 6 7 8 9 10; do
    echo ok
  done
done >> d.test

echo 1..00 > e.test
echo '1..000 # SKIP' > f.test

run_make -O TESTS='a.test b.test c.test d.test e.test f.test' check
count_test_results total=115 pass=113 xfail=0 skip=2 fail=0 xpass=0 error=0

:
