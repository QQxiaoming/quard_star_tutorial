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

# Test to make sure bison + bison's skeleton works.
# For Automake bug#7648 and PR automake/491.

required='cc bison'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_C_INLINE    dnl Required by some pre-C99 compilers such as MSVC.
AC_PROG_YACC
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = zardoz
zardoz_SOURCES = zardoz.y foo.c
AM_YFLAGS = -d --skeleton glr.c
BUILT_SOURCES = zardoz.h
END

# Parser.
cat > zardoz.y << 'END'
%{
int yylex ();
void yyerror (const char *s);
%}
%%
foobar : 'f' 'o' 'o' 'b' 'a' 'r' {};
%%
int yylex () { return 0; }
void yyerror (const char *s) { return; }
END

cat > foo.c << 'END'
#include "zardoz.h"
int main (void)
{
  return yyparse ();
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

# Try a VPATH build first.
mkdir build
cd build
../configure YACC='bison -y'
$MAKE
cd ..

# Now try an in-tree build.
./configure YACC='bison -y'
$MAKE

# Check that distribution is self-contained, and do not require
# bison to be built.
yl_distcheck YACC=false DISTCHECK_CONFIGURE_FLAGS='YACC=false'

:
