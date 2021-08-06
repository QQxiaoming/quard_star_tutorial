#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Regression test for bug when sources listed in conditional.
# Report from Richard Boulton.  PR/326.

. test-init.sh

cat >> configure.ac << 'END'
CC=false; AC_SUBST([CC])
OBJEXT=oo; AC_SUBST([OBJEXT])
AM_CONDITIONAL([ONE], [true])
AM_CONDITIONAL([TWO], [false])
AM_CONDITIONAL([THREE], [false])
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = targ

if ONE
SONE = one.c
endif

if TWO
STWO =
else
STWO = two.c
endif

if THREE
STHREE =
else
STHREE = three.c
endif

if THREE
STHREE2 =
else
STHREE2 = three2.c
endif

targ_SOURCES = $(SONE) $(STWO) $(STHREE) $(STHREE2)

.PHONY: test
test:
	is $(targ_OBJECTS) == one.oo two.oo three.oo three2.oo
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --ignore-deps
./configure
$MAKE test

:
