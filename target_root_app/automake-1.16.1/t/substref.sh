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

# Test for bug in variable substitution references when left hand
# pattern is null.
# Report from Richard Boulton.

. test-init.sh

cat >> configure.ac << 'END'
AC_SUBST([CC], [whocares])
AC_OUTPUT
END

: > hello.c

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = no-dependencies
var1 = dlmain
var2 = $(var1:=.)
helldl_SOURCES = $(var2:=c)
bin_PROGRAMS = helldl
.PHONY: test
test:
	is $(helldl_SOURCES) $(helldl_OBJECTS) == dlmain.c dlmain.$(OBJEXT)
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE test

# This is unrelated to the rest of this test.  But while we are
# at it, make sure we don't use am__helldl_SOURCES_DIST here, since
# it's not needed.  DIST_SOURCES should contain $(helldl_SOURCES).
grep am__helldl_SOURCES_DIST Makefile && exit 1
grep 'DIST_SOURCES.*\$(helldl_SOURCES)' Makefile

:
