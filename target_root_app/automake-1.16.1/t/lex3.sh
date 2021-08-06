#! /bin/sh
# Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

# Basic semantic checks on Lex support.
# Test associated with PR 19.
# From Matthew D. Langston.

required='cc lex'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_LEX
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_PROGRAMS = foo
foo_SOURCES = foo.l
END

cat > foo.l << 'END'
%{
#define YY_NO_UNISTD_H 1
%}
%%
"GOOD"   return EOF;
.
%%

int main (void)
{
  /* We don't use a 'while' loop here (like a real lexer would do)
     to avoid possible hangs. */
  if (yylex () == EOF)
    return 0;
  else
    return 1;
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

./configure

# Program should build and run.
$MAKE
if ! cross_compiling; then
  echo GOOD | ./foo
  echo BAD | ./foo && exit 1
  : For shells with busted 'set -e'.
fi

# The generated file 'foo.c' must be shipped.
$MAKE distdir
test -f $distdir/foo.c

# Sanity check on distribution.
yl_distcheck

# While we are at it, make sure that foo.c is erased by
# maintainer-clean, and not by distclean.
test -f foo.c
$MAKE distclean
test -f foo.c
./configure # Re-create 'Makefile'.
$MAKE maintainer-clean
test ! -e foo.c

:
