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

# This test checks that dependent files are updated before including
# in the distribution.  'lexer.c' depends on 'lexer.l'.  The latter is
# updated so that 'lexer.c' should be rebuild.  Then we are running
# 'make' and 'make distdir' and check whether the version of 'lexer.c'
# to be distributed is up to date.

# Please keep this in sync with sister test 'yaccvapth.sh'.

required='cc lex'
. test-init.sh

cat > lexoutroot.in << 'END'
LEX_OUTPUT_ROOT='@LEX_OUTPUT_ROOT@'
END

cat >> configure.ac << 'END'
AC_CONFIG_FILES([lexoutroot])
AC_PROG_CC
AC_PROG_LEX
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo
foo_SOURCES = lexer.l foo.c
LDADD = $(LEXLIB)
END

# Original lexer, with a "foobar" comment
cat > lexer.l << 'END'
%{
#define YY_NO_UNISTD_H 1
%}
%%
"END" return EOF;
.
%%
/*foobar*/
END

cat > foo.c << 'END'
int main (void)
{
  return 0;
}
/* Avoid possible link errors. */
int yywrap (void)
{
  return 1;
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

mkdir sub

# We must run configure early, to find out why $LEX_OUTPUT_ROOT is.
cd sub
../configure
. ./lexoutroot
test -n "$LEX_OUTPUT_ROOT" # Sanity check.
cd ..

$LEX lexer.l
mv "$LEX_OUTPUT_ROOT".c lexer.c

cd sub

# Ensure that lexer.l will be newer than lexer.c.
$sleep

# New lexer, with 'fubar' comment.
cat > ../lexer.l << 'END'
%{
#define YY_NO_UNISTD_H 1
%}
%%
"END" return EOF;
.
%%
/*fubar*/
END

$MAKE
$MAKE distdir
$FGREP '/*fubar*/' $distdir/lexer.c

#
# Now check to make sure that 'make dist' will rebuilt the parser.
#

# Ensure that lexer.l will be newer than lexer.c.
$sleep

# New lexer, with 'maude' comment.
cat > ../lexer.l << 'END'
%{
#define YY_NO_UNISTD_H 1
%}
%%
"END" return EOF;
.
%%
/*maude*/
END

$MAKE distdir
$FGREP '/*maude*/' $distdir/lexer.c

:
