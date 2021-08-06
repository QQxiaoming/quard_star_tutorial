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

# Check that the parallel test harness creates parent directories for
# the log files when needed.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = dir1/foo.test dir2/dir3/foo.test
TEST_LOG_COMPILER = sh
END

mkdir dir1 dir2 dir2/dir3
echo : > dir1/foo.test
echo : > dir2/dir3/foo.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

mkdir build
cd build
../configure
$MAKE check
find . # For debugging.
test -f test-suite.log
test -f dir1/foo.log
test -f dir1/foo.trs
test -f dir2/dir3/foo.log
test -f dir2/dir3/foo.trs

:
