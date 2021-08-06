#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Check that our concurrent test harness is not subject to spurious VPATH
# rewrites.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
LOG_COMPILER = true
TESTS = x0 foo bar baz x1
EXTRA_DIST = $(TESTS)
END

: > x0
: > foo
: > bar
: > baz
: > x1

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

mkdir build
cd build
../configure
$MAKE check
test -f test-suite.log
test -f foo.log
test -f foo.trs
test -f bar.log
test -f bar.trs
test -f baz.log
test -f baz.trs
# We don't want VPATH rewrites, really.
$MAKE check -n | $EGREP '\.\./(foo|bar|baz)( |$)' && exit 1
$MAKE distcheck

:
