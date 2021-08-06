#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Check parallel-tests features:
# - empty TESTS
# BSD make will expand '$(TESTS:=.log)' to '.log' unless overridden.
# See 'parallel-tests-trailing-whitespace.sh' for a similar issue.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS =
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
run_make -O check
for x in TOTAL PASS FAIL XPASS FAIL SKIP ERROR; do
  grep "^# $x: *0$" stdout
done

:
