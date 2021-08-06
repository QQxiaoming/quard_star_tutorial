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

# Auxiliary test to set up common data used by many tests on TAP support.

. test-init.sh

cat >> configure.ac << END
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_LOG_DRIVER = $(srcdir)/tap-driver
TEST_LOG_COMPILER = cat
TESTS = all.test
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

rm -rf autom4te*.cache

# So that the data files we've created won't be removed at exit.
keep_testdirs=yes

:
