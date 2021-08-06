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
#  - TODO and SKIP directives are case-insensitive
#  - TODO and SKIP directives can be followed optionally by a colon ":"
#    and by an optional explanation.
#  - our driver isn't fooled into recognizing TODO and SKIP directives
#    spuriously
#  - the reasons for TODO and SKIP, if present, are nicely printed in
#    the testsuite progress output

. test-init.sh

. tap-setup.sh

# ----------------------------------------------------- #
#  Check all possible combinations of:                  #
#    - uppercase/lowercase                              #
#    - with/without colon character ":"                 #
#    - with/without explanatory message                 #
#  in TODO and SKIP directives.                         #
# ----------------------------------------------------- #

# There are 2 * 2^6 + 2 * 2^6 = 256 tests.
echo 1..256 > all.test

# These nested loops are clearer without indentation.

for c1 in t T; do
for c2 in o O; do
for c3 in d D; do
for c4 in o O; do
for ex in '' ':' ' foo' ': foo'; do
  echo "not ok # $c1$c2$c3$c4$ex"
  echo "not ok# $c1$c2$c3$c4$ex"
done; done; done; done; done >> all.test

for c1 in s S; do
for c2 in k K; do
for c3 in i I; do
for c4 in p P; do
for ex in '' ':' ' foo' ': foo'; do
  echo "ok # $c1$c2$c3$c4$ex"
  echo "ok# $c1$c2$c3$c4$ex"
done; done; done; done; done >> all.test

cat all.test # For debugging.

run_make -O check
count_test_results total=256 pass=0 fail=0 xpass=0 xfail=128 skip=128 error=0

# -------------------------------------------------------- #
#  TODO ans SKIP directives aren't recognized spuriously.  #
# -------------------------------------------------------- #

cat > all.test <<'END'
1..9
ok TODO
ok - TODO
ok 3 TODO
ok 4 - TODO
ok SKIP
ok - SKIP
ok 7 SKIP
ok 8 - SKIP
ok 9
END

run_make -O check
count_test_results total=9 pass=9 fail=0 xpass=0 xfail=0 skip=0 error=0

:
