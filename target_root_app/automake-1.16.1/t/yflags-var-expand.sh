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

# Check that automake expand variables when looking for '-d' in YFLAGS;
# for example, the following is supposed to work:
#  foo_flags = -d
#  AM_YFLAGS = $(foo_flags)

. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AC_PROG_YACC
END

$ACLOCAL

cat > Makefile.am <<'END'
bin_PROGRAMS = foo bar
foo_SOURCES = foo.y
bar_SOURCES = bar.y
my_YFLAGS = -x
AM_YFLAGS = $(my_YFLAGS:x=d)
bar_YFLAGS = $(AM_YFLAGS)
END

$AUTOMAKE -a

$EGREP '(foo|bar|YFLAGS)' Makefile.in # For debugging.
grep '^foo.h *:' Makefile.in
grep '^bar-bar.h *:' Makefile.in

cat > Makefile.am <<'END'
AUTOMAKE_OPTIONS = -Wno-gnu
bin_PROGRAMS = zardoz
zardoz_SOURCES = parser.y
my_YFLAGS = $(my_YFLAGS_1)
my_YFLAGS += $(my_YFLAGS_2)
my_YFLAGS_2 = -d
YFLAGS = $(my_YFLAGS)
END

$AUTOMAKE

$EGREP 'parser|YFLAGS' Makefile.in # For debugging.
grep '^parser.h *:' Makefile.in

:
