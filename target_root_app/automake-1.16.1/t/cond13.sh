#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Test for bug in conditionals.
# Report from Lars J. Aas.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_CXX
AM_PROG_AR
AC_PROG_RANLIB
AM_CONDITIONAL([COND1], [true])
AM_CONDITIONAL([COND2], [true])
END

cat > Makefile.am << 'END'
noinst_LIBRARIES = libtest.a

if COND1
SOURCEVAR1 =
SOURCEVAR2 = habla.cpp espanol.cpp
else
SOURCEVAR1 = dummy.cpp
SOURCEVAR2 =
endif

if COND2
TESTSOURCES = $(SOURCEVAR1)
else
TESTSOURCES = $(SOURCEVAR2)
endif

libtest_a_SOURCES = $(TESTSOURCES)
END

: > ar-lib

$ACLOCAL
$AUTOMAKE

grep '^am_libtest_a_OBJECTS =' Makefile.in

:
