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

# Check that automake can cope with a definition of the $(YFLAGS) variable
# in Makefile.am (even if that is extremely bad practice, because that
# variable is user-reserved).

required='cc yacc'
. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AC_PROG_YACC
AC_OUTPUT
END

cat > Makefile.am <<'END'
bin_PROGRAMS = foo
foo_SOURCES = foo.y
# Don't do this in a real-life Makefile.am!
YFLAGS = -d -v
END

cat > foo.y << 'END'
%{
int yylex () { return 0; }
void yyerror (char *s) { return; }
int main () { return 0; }
%}
%%
foobar : 'f' 'o' 'o' 'b' 'a' 'r' {};
END

$ACLOCAL
$AUTOMAKE -a -Wno-gnu

$EGREP '(foo|YFLAGS)' Makefile.in # For debugging.
grep '^foo\.h *:' Makefile.in

$AUTOCONF
./configure

$MAKE

test -f foo.c
test -f foo.h
test -f foo.output

:
