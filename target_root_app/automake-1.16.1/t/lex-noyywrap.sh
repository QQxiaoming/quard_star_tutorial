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

# Check Lex support with flex using the '%noyywrap' option.

required='cc flex'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_LEX
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo
foo_SOURCES = foo.l

.PHONY: test-no-lexlib
check-local: test-no-lexlib
test-no-lexlib:
	test x'$(LEXLIB)' = x'none needed'
END

cat > foo.l << 'END'
%option noyywrap
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

./configure LEXLIB="none needed"

# Program should build and run.
$MAKE
if ! cross_compiling; then
  echo GOOD | ./foo
  echo BAD | ./foo && exit 1
  : For shells with busted 'set -e'.
fi

# Sanity check on distribution.  Escape in LEXLIB must use backspace,
# not double-quotes, to avoid a spurious failure with AIX make.
yl_distcheck DISTCHECK_CONFIGURE_FLAGS='LEXLIB=none\ needed'

:
