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

# Automake lex support can work with flex '--header-file' option (see
# bugs #8844 and #9933).

required='cc flex'
. test-init.sh

# Here, we need to use the use flex option '--header-file', but some
# older flex versions don't support is (see automake bug#11524 and
# bug#12836).  Skip this test if such an old flex version is detected.
$LEX --help | grep '.*--header-file' \
  || skip_ "flex doesn't support the '--header-file' option"

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_LEX
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo
foo_SOURCES = lexer.l main.c mylex.h
foo_LFLAGS = --header-file=mylex.h
BUILT_SOURCES = mylex.h
# Recover from removal of header.
mylex.h: foo-lexer.c
	test -f $@ || rm -f foo-lexer.c
	test -f $@ || $(MAKE) $(AM_MAKEFLAGS) foo-lexer.c
END

cat > lexer.l << 'END'
%option noyywrap
%{
#define YY_NO_UNISTD_H 1
%}
%%
"GOOD"   return EOF;
.
%%
END

cat > main.c <<'END'
#define YY_NO_UNISTD_H 1
#include "mylex.h"
int main (void)
{
  /* We don't use a 'while' loop here (like a real lexer would do)
     to avoid possible hangs. */
  if (yylex () == EOF)
    return 0;
  else
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

# Recovering from header removal.
rm -f mylex.h
$MAKE
test -f mylex.h

# Sanity check on distribution.
yl_distcheck

:
