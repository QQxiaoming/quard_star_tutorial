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

# Regression test for substitution references to conditional variables.
# Report from Richard Boulton.

. test-init.sh

cat >> configure.ac << 'END'
AC_SUBST([CC], [false])
AC_SUBST([OBJEXT], [o])
AM_CONDITIONAL([COND1], [test "x$CONDITION1" = "xtrue"])
AM_CONDITIONAL([COND2], [test "x$CONDITION2" = "xtrue"])
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = helldl

var1 = dlmain

if COND1
var2 = $(var1:=.c)
else
var2 = $(var1:=.c)
endif

if COND2
var3 = $(var2:.c=a.c)
var4 = $(var2:.c=b.c)
else
var3 = $(var2:.c=b.c)
var4 = $(var2:.c=a.c)
endif

helldl_SOURCES = $(var3:.c=1.c) $(var4:.c=2.c)

.PHONY: test
test:
	is $(exp) == $(helldl_SOURCES) $(helldl_OBJECTS)
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a -i

CONDITION1=true CONDITION2=true ./configure
$MAKE test exp='dlmaina1.c dlmainb2.c dlmaina1.o dlmainb2.o'
CONDITION1=true CONDITION2=false ./configure
$MAKE test exp='dlmainb1.c dlmaina2.c dlmainb1.o dlmaina2.o'
CONDITION1=false CONDITION2=true ./configure
$MAKE test exp='dlmaina1.c dlmainb2.c dlmaina1.o dlmainb2.o'
CONDITION1=false CONDITION2=false ./configure
$MAKE test exp='dlmainb1.c dlmaina2.c dlmainb1.o dlmaina2.o'

:
