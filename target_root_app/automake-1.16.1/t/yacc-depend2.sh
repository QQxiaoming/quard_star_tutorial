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

# Make sure depcomp does not needlessly update headers and objects
# for yacc rules.  This test still fails with FreeBSD make (but passes
# with NetBSD make).

required='cc yacc'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_YACC
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo
AM_YFLAGS = -d
foo_SOURCES = foo.y main.c
BUILT_SOURCES = foo.h
.PHONY: test-time-unchanged test-time-changed
test-time-unchanged:
	is_newest foo.y foo.h main.$(OBJEXT)
test-time-changed:
	is_newest main.$(OBJEXT) foo.y foo.h
END

cat > foo.y << 'END'
%{
int yylex () { return 0; }
void yyerror (char *s) { return; }
%}
%token TOKEN
%%
foobar : 'f' 'o' 'o' 'b' 'a' 'r' {};
END


cat > main.c << 'END'
#include "foo.h"
int main(void)
{
  return yyparse ();
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

# Try to enable dependency tracking if possible, even if that means
# using slow dependency extractors.
./configure --enable-dependency-tracking
$MAKE
ls -l # For debugging.

$sleep
touch foo.y
$MAKE
$MAKE test-time-unchanged
$sleep
sed 's/TOKEN/TEKON/g' foo.y > t
mv -f t foo.y
$MAKE
$MAKE test-time-changed

:
