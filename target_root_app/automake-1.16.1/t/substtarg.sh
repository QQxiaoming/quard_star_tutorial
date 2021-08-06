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

# Test that substitutions in variables work.
# From Lars J. Aas.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_SOURCE([fakelib.c])
AC_PROG_CC
AM_PROG_AR
RANLIB=:
AC_SUBST([RANLIB])
SUBST=hei
AC_SUBST([SUBST])
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_LIBRARIES = libfake@SUBST@.a

libfake@SUBST@_a_SOURCES = abra.c kadabra.c

# Then we override the target rule:
libfake@SUBST@.a: Makefile $(libfake@SUBST@_a_OBJECTS) $(libfake@SUBST@_a_DEPENDENCIES)
	@echo here we do some custom stuff, instead of invoking the linker
END

: > ar-lib

$ACLOCAL
AUTOMAKE_fails
grep 'overrid.*libfake@SUBST@.a' stderr
$AUTOMAKE -Wno-override
test $(grep -c '^libfake@SUBST@.a:' Makefile.in) -eq 1

:
