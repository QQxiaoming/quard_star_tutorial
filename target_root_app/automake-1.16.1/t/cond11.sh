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

# Test for bug in conditionals.  From Richard Boulton.
# This checks that, if LDADD is set from a conditional variable
# and an AC_SUBST, the _DEPENDENCIES variable is set correctly.

. test-init.sh

cat >> configure.ac << 'END'
AC_SUBST([CC], [false])
AM_CONDITIONAL([USE_A], [test -z "$two"])
AC_SUBST([SUBSTVAR], [bar])
AC_OUTPUT
END

cat > Makefile.am << 'END'
if USE_A
foolibs=faz.la
else
foolibs=
endif

noinst_PROGRAMS = foo
foo_SOURCES = foo.c
LDADD = $(SUBSTVAR) $(foolibs)

.PHONY: test1 test2
test1:
	is faz.la == $(foo_DEPENDENCIES)
test2:
	is "" == $(foo_DEPENDENCIES)
END

: > config.guess
: > config.sub

$ACLOCAL
$AUTOCONF
$AUTOMAKE --ignore-deps

./configure
$MAKE test1

./configure two=yes
$MAKE test2

:
