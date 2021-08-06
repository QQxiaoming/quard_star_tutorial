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

# Check that yacc sources with many dots in their name are handled
# correctly.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_CXX
AC_PROG_YACC
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo bar foo2 bar2

foo_SOURCES = parse.y.y
bar_SOURCES = parse.s.f..y
bar_YFLAGS = -d

foo2_SOURCES = parse..5.y++
bar2_SOURCES = parse.yxx.yy
bar2_YFLAGS = -d
END

outputs=' parse.y.c      bar-parse.s.f..c   bar-parse.s.f..h
          parse..5.c++   bar2-parse.yxx.cc  bar2-parse.yxx.hh '

$ACLOCAL
$AUTOMAKE -a

$EGREP '(\.[ch]|parse)' Makefile.in # For debugging.

# All expected files should be mentioned in the generated Makefile.in.
for s in $outputs; do
  $FGREP $s Makefile.in
done

:
