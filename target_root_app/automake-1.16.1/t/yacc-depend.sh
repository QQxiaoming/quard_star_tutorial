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

# Make sure depcomp does not needlessly update headers for yacc rules.
# Report from Paolo Bonzini.

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

# Make sure foo.h is not updated if not really needed.
$sleep
: > my-timestamp
$sleep
touch foo.y
$MAKE
stat my-timestamp foo.* || : # For debugging.
is_newest my-timestamp foo.h

# Make sure foo.h is updated if needed.
$sleep
sed 's/TOKEN/TEKON/g' foo.y > t
mv -f t foo.y
$MAKE
stat my-timestamp foo.* || : # For debugging.
is_newest foo.h my-timestamp

:
