#! /bin/sh
# Copyright (C) 2004-2018 Free Software Foundation, Inc.
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

# Check the filename-length-max=99 option.

. test-init.sh

echo AC_OUTPUT >> configure.ac

cat >Makefile.am <<'END'
AUTOMAKE_OPTIONS = filename-length-max=99
EXTRA_DIST = 12345678
END

(for i in 1 2 3 4 5 6 7 8 9; do
  mkdir -p 12345678 && cd 12345678 && touch x || exit 1
done) || skip_ "failed to create deep directory hierarchy"

# AIX 5.3 'cp -R' is too buggy for 'make dist'.
cp -R 12345678 t \
  || skip_ "'cp -R' failed to copy deep directory hierarchy"

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure
$MAKE distcheck

(for i in 1 2 3 4 5 6 7 8 9 10 11; do
  mkdir -p 12345678 && cd 12345678 && touch x || exit 1
done) || skip_ "failed to create deeper directory hierarchy"

run_make -E -e FAIL dist
grep 'filenames are too long' stderr
test 2 -eq $(grep -c 12345678 stderr)

:
