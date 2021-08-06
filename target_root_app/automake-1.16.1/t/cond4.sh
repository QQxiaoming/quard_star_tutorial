#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Another sources-in-conditional test.  Report from Tim Goodwin.

. test-init.sh

cat >> configure.ac << 'END'
AC_SUBST([CC], [false])
AC_SUBST([OBJEXT], [o])
AM_CONDITIONAL([ONE], [test "x$CONDITION1" = "xtrue"])
AM_CONDITIONAL([TWO], [test "x$CONDITION2" = "xtrue"])
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = targ

if ONE
OPT1 = one.c
endif

if TWO
OPT2 = two.c
endif

targ_SOURCES = main.c $(OPT1) $(OPT2)

.PHONY: test
test:
	is $(exp) == $(targ_OBJECTS)
END

$ACLOCAL
$AUTOMAKE -i

# We should not output useless definitions.
grep '^@ONE_FALSE@' Makefile.in && exit 1
grep '^@TWO_FALSE@' Makefile.in && exit 1

$AUTOCONF

CONDITION1=true CONDITION2=true ./configure
$MAKE test exp='main.o one.o two.o'
CONDITION1=true CONDITION2=false ./configure
$MAKE test exp='main.o one.o'
CONDITION1=false CONDITION2=true ./configure
$MAKE test exp='main.o two.o'
CONDITION1=false CONDITION2=false ./configure
$MAKE test exp='main.o'

:
