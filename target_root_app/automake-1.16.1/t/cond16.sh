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

# Test for bug in conditionals in SOURCES with variable substitution
# references.
# Report from Richard Boulton.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([COND1], [true])
AC_OUTPUT
END

: > hello.c

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = no-dependencies
CC = false
OBJEXT = o

if COND1
var = foo.c
else
var = foo.c
endif

bin_PROGRAMS = hell
hell_SOURCES = $(var:=)

.PHONY: test
test:
	is $(hell_SOURCES) $(hell_OBJECTS) == foo.c foo.o
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
./configure
$MAKE test

:
