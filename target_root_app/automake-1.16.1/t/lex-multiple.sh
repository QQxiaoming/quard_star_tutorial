#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Check that we can build a program using several lexers at once
# (assuming Flex is used).  That is a little tricky, but possible.
# See:
# <https://lists.gnu.org/archive/html/automake/2010-10/msg00081.html>
# <https://lists.gnu.org/archive/html/automake/2009-03/msg00061.html>

required='cc flex'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_LEX
AM_PROG_AR
AC_PROG_RANLIB
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = zardoz

zardoz_SOURCES = main.c
# Convenience libraries.
noinst_LIBRARIES = liblex.a liblex-foo.a liblex-bar.a
zardoz_LDADD = $(noinst_LIBRARIES)

liblex_a_SOURCES = 0.l

# We need the output to always be named 'lex.yy.c', in order for
# ylwrap to pick it up.
liblex_foo_a_LFLAGS = -Pfoo -olex.yy.c
liblex_foo_a_SOURCES = a.l

# Ditto.
liblex_bar_a_LFLAGS = -Pbar_ -olex.yy.c
liblex_bar_a_SOURCES = b.l
END

cat > main.c << 'END'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv[])
{
  if (argc != 2)
    abort ();
  else if (!strcmp(argv[1], "--vanilla"))
    return (yylex () != 121);
  else if (!strcmp(argv[1], "--foo"))
    return (foolex () != 121);
  else if (!strcmp(argv[1], "--bar"))
    return (bar_lex () != 121);
  else
    abort ();
}
END

cat > 0.l << 'END'
%{
#define YY_NO_UNISTD_H 1
%}
%%
"VANILLA" { printf (":%s:\n", yytext); return 121; }
. { printf (":%s:\n", yytext); return 1; }
%%
/* Avoid possible link errors. */
int yywrap (void) { return 1; }
END

sed 's/VANILLA/FOO/' 0.l > a.l
sed 's/VANILLA/BAR/' 0.l > b.l

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure
$MAKE

if ! cross_compiling; then
  echo VANILLA | ./zardoz --vanilla
  echo FOO | ./zardoz --foo
  echo BAR | ./zardoz --bar
  ./zardoz --vanilla </dev/null && exit 1
  echo BAR | ./zardoz --foo && exit 1
  : For shells with busted 'set -e'.
fi

yl_distcheck

:
