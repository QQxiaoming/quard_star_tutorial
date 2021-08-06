#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Check that our fake "C compiler" that doesn't grasp the '-c' and
# '-o' command-line options passed together, used to enhance testsuite
# coverage.

required=gcc # Our fake compiler uses gcc.
am_create_testdir=empty
. test-init.sh

CC=$am_testaux_builddir/cc-no-c-o; export CC

echo 'int main (void) { return 0; }' > foo.c
$CC -c foo.c
test -f foo.o || test -f foo.obj
$CC -c -o bar.o foo.c 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
grep "both '-o' and '-c' seen on the command line" stderr
test ! -e bar.o && test ! -e bar.obj

:
