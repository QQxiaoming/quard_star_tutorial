#! /bin/sh
# Copyright (C) 2005-2018 Free Software Foundation, Inc.
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

# Make sure ##-comments are ignored in variable definitions.
# Report from Julien Sopena.

. test-init.sh

cat > Makefile.am << 'EOF'
TESTS = \
   1.test \
   2.test \
   3.test \
## 4.test \
   5.test \
   6.test \
   7.test
EOF

: > test-driver

$ACLOCAL
$AUTOMAKE

sed -n -e '/^TESTS =.*\\$/ {
   :loop
   p
   n
   t clear
   :clear
   s/\\$/\\/
   t loop
   p
   n
   }' -e '/^TESTS =/ p' Makefile.in > tests

grep '3\.test' tests
grep '##' tests && exit 1
grep '4\.test' tests && exit 1
grep '5\.test' tests
