#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Removal recovery rules for headers should not remove files with 'make -n'.

required='cc yacc'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_YACC
AC_OUTPUT
END

cat > Makefile.am << 'END'
AM_YFLAGS = -d
bin_PROGRAMS = foo
foo_SOURCES = foo.c parse.y
END

cat > foo.c << 'END'
int main () { return 0; }
END

cat > parse.y << 'END'
%{
int yylex () { return 0; }
void yyerror (char *s) {}
%}
%%
foobar : 'f' 'o' 'o' 'b' 'a' 'r' {};
END

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF
./configure
$MAKE

rm -f parse.h
$MAKE -n parse.h
test -f parse.c
test ! -e parse.h

:
