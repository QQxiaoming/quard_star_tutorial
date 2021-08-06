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

# parallel-tests:
#   - check2.am and interaction with $(EXEEXT)

. test-init.sh

cat >> configure.ac << 'END'
dnl We need to fool the init.m4 internals a little.
AC_DEFUN([_AM_FOO],
  [m4_provide([_AM_COMPILER_EXEEXT])
  AC_SUBST([CC], [false])
  AC_SUBST([EXEEXT])])
_AM_FOO
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = x y a.test b.test
LOG_COMPILER = true
TEST_LOG_COMPILER = true
## We also need to fool the automake internals a little.
EXTRA_PROGRAMS = y
y_SOURCES =
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

$EGREP 'EXEEXT|\.log|\.test' Makefile.in # For debugging.

./configure EXEEXT=
touch x y a.test b.test
$MAKE check

$MAKE distclean
rm -f x y a.test b.test *.log *.trs

./configure EXEEXT=.bin
touch x y.bin a.test b.test.bin
$MAKE check
ls -l # For debugging.
test -f y.log
test ! -e y.bin.log
test -f b.log
test ! -e b.test.log

$EGREP '^y\.log: y(\$\(EXEEXT\)|\.bin)' Makefile
$EGREP '^\.test(\$\(EXEEXT\)|\.bin)\.log:' Makefile

:
