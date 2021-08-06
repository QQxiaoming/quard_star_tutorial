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

# parallel-tests support: the following are registered with '.SUFFIXES':
#  - .log
#  - .trs (used by files that store test results and metadata)
#  - .test if $(TEST_EXTENSIONS) is not defined
#  - stuff in $(TEST_EXTENSIONS) otherwise

. test-init.sh

: > Makefile.am

cat > 1.am << 'END'
TESTS =
END

cat > 2.am << 'END'
TEST_EXTENSIONS = .SH .abcdef
TESTS =
END

: > test-driver

$ACLOCAL

$AUTOMAKE 1
$AUTOMAKE 2

sed -e 's/$/ /' 1.in > mk.1
sed -e 's/$/ /' 2.in > mk.2

grep '^\.SUFFIXES:' mk.1
grep '^\.SUFFIXES:' mk.2

for suf in test log trs; do
  grep "^\\.SUFFIXES:.* \\.$suf " mk.1
done

for suf in SH abcdef log trs; do
  grep "^\\.SUFFIXES:.* \\.$suf " mk.2
done

:
