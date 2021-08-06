#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Test for subdir lexers.

required='cc lex'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_LEX
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS  = subdir-objects
LDADD             = @LEXLIB@

bin_PROGRAMS    = foo/foo
foo_foo_SOURCES = foo/foo.l
END

mkdir foo

cat > foo/foo.l << 'END'
%{
#define YY_NO_UNISTD_H 1
%}
%%
"END"   return EOF;
.
%%
int
main ()
{
  while (yylex () != EOF)
    ;

  return 0;
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

# We expect ylwrap to be used and distributed even if there is
# only one lexer.
test -f ylwrap

mkdir sub
cd sub
../configure
$MAKE foo/foo.o
ls -l # For debugging.

test -f foo/foo.c
test -f foo/foo.o

# Now, adds another lexer to test ylwrap.

cd ..
cp foo/foo.l foo/foo2.l
cat >> Makefile.am << 'END'
EXTRA_foo_foo_SOURCES = foo/foo2.l
END

# Make sure Makefile.in has a new time stamp: the rebuild rules are
# used below.  We do this after updating Makefile.am, that way we can
# ensure that automake, even with --no-force, is not confused if the
# new Makefile.am has the same time stamp as the older one (since the
# output will change, --no-force should have no effect).
$sleep

$AUTOMAKE -a --no-force

cd sub
using_gmake || $MAKE Makefile
$MAKE foo/foo2.o
ls -l # For debugging.
test -f foo/foo2.c
test -f foo/foo2.o

:
