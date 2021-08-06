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

# Basic semantic checks on Yacc support (without yacc-generated headers).
# Keep in sync with sister test 'yacc-cxx.sh'.

required='cc yacc'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_YACC
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo bar
foo_SOURCES = parse.y foo.c
bar_SOURCES = $(foo_SOURCES)
bar_YFLAGS = -v

.PHONY: echo-distcom
echo-distcom:
	@echo ' ' $(DIST_COMMON) ' '
END

cat > parse.y << 'END'
%{
#include <stdio.h>
#include <stdlib.h>
int yylex () { return getchar (); }
void yyerror (char *s) {}
%}
%%
a : 'a' { exit(0); };
END

cat > foo.c << 'END'
int main () { yyparse (); return 1; }
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE
ls -l
# The Yacc-derived C sources must be created, and not removed once
# compiled (i.e., not treated like "intermediate files" in the GNU
# make sense).
test -f parse.c
test -f bar-parse.c
# Check that per-object flags are honored.
test -f bar-parse.output

if ! cross_compiling; then
  echo a | ./foo
  echo b | ./foo && exit 1
  echo a | ./bar
  echo b | ./bar && exit 1
  : For shells with busted 'set -e'.
fi

# The Yacc-derived C sources must be shipped.
$MAKE echo-distcom
$MAKE -s echo-distcom | grep '[ /]parse\.c '
$MAKE -s echo-distcom | grep '[ /]bar-parse\.c '
$MAKE distdir
ls -l $distdir
test -f $distdir/parse.c
test -f $distdir/bar-parse.c

# Sanity check on distribution.
# Note that, for this to succeed, bar-parse.output must either not
# be distributed, or properly cleaned by automake-generated rules.
# We don't want to set the exact semantics yet, but want to ensure
# they are are consistent.
yl_distcheck

# Make sure that the Yacc-derived C sources are erased by
# maintainer-clean, and not by distclean.
test -f parse.c
test -f bar-parse.c
$MAKE distclean
ls -l
test -f parse.c
test -f bar-parse.c
./configure # We must re-create 'Makefile'.
$MAKE maintainer-clean
ls -l
test ! -e parse.c
test ! -e bar-parse.c

:
