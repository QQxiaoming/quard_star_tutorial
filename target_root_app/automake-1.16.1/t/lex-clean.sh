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

# Check that .c files derived from non-distributed .l sources
# are cleaned by "make clean", while .c files derived from
# distributed .l sources are cleaned by "make maintainer-clean".
# See also sister test 'lex-clean-cxx.sh'.

required='cc lex'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_LEX
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo bar baz qux

foo_SOURCES = main.c lexer.l

bar_SOURCES = main.c lexer.l
bar_LFLAGS = $(AM_LFLAGS)

baz_SOURCES = main.c
nodist_baz_SOURCES = baz.l

qux_SOURCES = main.c
nodist_qux_SOURCES = baz.l
qux_LFLAGS = $(AM_LFLAGS)

baz.l:
	cp $(srcdir)/lexer.l $@

CLEANFILES = baz.l

LDADD = $(LEXLIB)
END

cat > lexer.l << 'END'
%{
#define YY_NO_UNISTD_H 1
%}
%%
"GOOD"   return EOF;
.
END

cat > main.c << 'END'
int main (void)
{
  return yylex ();
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

cp config.status config.sav

$MAKE
ls -l
# Sanity checks.
test -f lexer.l
test -f lexer.c
test -f bar-lexer.c
test -f baz.l
test -f baz.c
test -f qux-baz.c

for target in clean distclean; do
  $MAKE $target
  ls -l
  test -f lexer.l
  test -f lexer.c
  test -f bar-lexer.c
  test ! -e baz.l
  test ! -e baz.c
  test ! -e qux-baz.c
done

cp config.sav config.status
./config.status # re-create Makefile

$MAKE maintainer-clean
ls -l
test -f lexer.l
test ! -e lexer.c
test ! -e bar-lexer.c

:
