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

# This test checks that dependent files are updated before including
# in the distribution. 'parse.c' depends on 'parse.y'. The later is
# updated so that 'parse.c' should be rebuild. Then we are running
# 'make' and 'make distdir' and check whether the version of 'parse.c'
# to be distributed is up to date.

# Please keep this in sync with sister test 'yacc-d-vpath.sh'.

required='cc yacc'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_YACC
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo
foo_SOURCES = parse.y foo.c
END

# Original parser, with 'foobar'.
cat > parse.y << 'END'
%{
int yylex () { return 0; }
void yyerror (char *s) {}
%}
%%
foobar : 'f' 'o' 'o' 'b' 'a' 'r' {};
END

cat > foo.c << 'END'
int main () { return 0; }
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

$YACC parse.y
mv y.tab.c parse.c

mkdir sub
cd sub
../configure

$sleep

# New parser, with 'fubar'.
cat > ../parse.y << 'END'
%{
int yylex () { return 0; }
void yyerror (char *s) {}
%}
%%
fubar : 'f' 'o' 'o' 'b' 'a' 'r' {};
END

$MAKE
$MAKE distdir
$FGREP fubar $distdir/parse.c

# Now check to make sure that 'make dist' will rebuild the parser.

$sleep

# New parser, with 'maude'.
cat > ../parse.y << 'END'
%{
int yylex () { return 0; }
void yyerror (char *s) {}
%}
%%
maude : 'm' 'a' 'u' 'd' 'e' {};
END

$MAKE distdir
$FGREP maude $distdir/parse.c

:
