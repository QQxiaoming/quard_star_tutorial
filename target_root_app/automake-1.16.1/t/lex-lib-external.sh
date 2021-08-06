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

# Check that we can get use the 'yywrap' function from a system-wide
# library, if that's available.

required='cc lex'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_LEX
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = lexer
lexer_SOURCES = foo.l
lexer_LDADD = $(LEXLIB)

.PHONY: have-lexlib
have-lexlib:
	test x'$(LEXLIB)' != x
	echo 'int main (void) { return yywrap (); }' > x.c
	$(CC) -c x.c
	$(CC) x.$(OBJEXT) $(LEXLIB)
	rm -f x.c *.$(OBJEXT) *.o *.out *.exe
END

cat > foo.l <<'END'
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
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE have-lexlib || skip_ "no system-wide lex library found"

# Program should build and run and distribute.
$MAKE all
if ! cross_compiling; then
  echo GOOD | ./lexer
  echo BAD | ./lexer && exit 1
  : For shells with busted 'set -e'.
fi
yl_distcheck

:
