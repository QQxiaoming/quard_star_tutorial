#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Test for bug in variable substitution references, where
# undefined variables break later substitutions.

. test-init.sh

cat >> configure.ac << 'END'
AC_SUBST([CC], [whocares])
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = no-dependencies

foo = foo.a foo.b $(doesnt_exist)
bar = bar.a bar.b
var1 = $(foo:.a=1.c) $(doesnt_exist:.b=2.c) $(bar:.a=3.c)
var2 = $(var1:.b=4.c)

bin_PROGRAMS = foo
foo_SOURCES = $(var2)

OBJEXT = obj
.PHONY: test
test:
	is $(foo_OBJECTS) == foo1.obj foo4.obj bar3.obj bar4.obj
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
./configure
$MAKE test

:
