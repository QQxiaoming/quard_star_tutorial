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

# Check that we can provide a personal 'yywrap' function in a custom
# library.
# See also test 'lex-lib-external.sh'.

required='cc lex'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
LEXLIB=libmylex.a
AC_PROG_LEX
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = lexer
lexer_SOURCES = foo.l
lexer_LDADD = $(LEXLIB)
EXTRA_lexer_DEPENDENCIES = $(LEXLIB)
noinst_LIBRARIES = libmylex.a
libmylex_a_SOURCES = mu.c
END

cat > mu.c << 'END'
int yywrap (void)
{
  return 1;
}
END

cat > foo.l <<'END'
%{
#define YY_NO_UNISTD_H 1
%}
%%
"END" return EOF;
.
%%
int main (void)
{
  return 0;
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE
test -f foo.c
test -f libmylex.a

:
