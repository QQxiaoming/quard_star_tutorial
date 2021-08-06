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
#  - "SKIP" keyword in a TAP plan is case-insensitive

. test-init.sh

. tap-setup.sh

# These nested loops below are clearer without indentation.
i=0
for c1 in s S; do
for c2 in k K; do
for c3 in i I; do
for c4 in p P; do
  i=$(($i + 1))
  case $i in ?) j=0$i;; *) j=$i;; esac
  echo "1..0 # $c1$c2$c3$c4 foobar" > $j.test
done; done; done; done

run_make -O TESTS="$(echo *.test)" check
count_test_results total=16 pass=0 fail=0 xpass=0 xfail=0 skip=16 error=0

for tst in *.test; do
  grep "^SKIP: $tst .* foobar$" stdout
done

:
