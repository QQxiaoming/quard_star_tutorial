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

# parallel-tests: check that we can use variables and functions set
# by TESTS_ENVIRONMENT and AM_TESTS_ENVIRONMENT in LOG_COMPILER and
# LOG_FLAGS (for tests both with and without registered extensions).

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

unset v0 v1 v2 v3 v4

cat > Makefile.am << 'END'
TESTS_ENVIRONMENT = am__f3 () { echo 3; }; v0='$(srcdir)' v1=1; :;
AM_TESTS_ENVIRONMENT = v2=2; v4=4 && am__f5 () { echo 5; }; :;

LOG_COMPILER = $$v0/log-compiler $$v1
AM_LOG_FLAGS = $$v2 `am__f3`
LOG_FLAGS = $$v4 `am__f5`

TEST_LOG_COMPILER = $$v0/test-log-compiler $$v2
AM_TEST_LOG_FLAGS = `am__f5` $$v1
TEST_LOG_FLAGS = $$v4 `am__f3`

EXTRA_DIST = log-compiler test-log-compiler

TESTS = a b.test
$(TESTS):
END

cat > log-compiler << 'END'
#! /bin/sh
echo "$0: $*"
test x"$1$2$3$4$5" = x"12345"
END

cat > test-log-compiler << 'END'
#! /bin/sh
echo "$0: $*"
test x"$1$2$3$4$5" = x"25143"
END

chmod a+x test-log-compiler log-compiler

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

$MAKE -n check || : # For debugging.
VERBOSE=yes $MAKE check
cat a.log
cat b.log

VERBOSE=yes $MAKE distcheck

:
