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

# Check that VPATH builds and "make distcheck" works with packages
# using yacc and the automake 'subdir-objects' option.
# Exposes automake bug#8485.

required='cc yacc'
. test-init.sh

# This test is bounded to fail for any implementation that
# triggers automake bug#7884.
useless_vpath_rebuild && skip_ "would trip on automake bug#7884"

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_YACC
AC_OUTPUT
END

mkdir sub

cat > sub/parse.y << 'END'
%{
int yylex () { return 0; }
void yyerror (char *s) { return; }
%}
%%
x : 'x' {};
%%
int main (void)
{
  return yyparse ();
}
END

cat > Makefile.am <<'END'
AUTOMAKE_OPTIONS = subdir-objects
noinst_PROGRAMS = foo bar
foo_SOURCES = sub/parse.y
bar_SOURCES = $(foo_SOURCES)
AM_YFLAGS = -d
bar_YFLAGS =
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE distdir

# Yacc-derived C source and header files must be built and distributed.

test   -f sub/parse.c
test   -f sub/parse.h
test   -f sub/bar-parse.c
test ! -e sub/bar-parse.h

test   -f $distdir/sub/parse.c
test   -f $distdir/sub/parse.h
test   -f $distdir/sub/bar-parse.c
test ! -e $distdir/sub/bar-parse.h

# But they shouldn't be rebuilt in VPATH builds.

mkdir $distdir/build
chmod -R a-w $distdir
cd $distdir/build
chmod u+w .
# Try to enable dependency tracking even with slow dependency
# extractors, to improve coverage.
../configure --enable-dependency-tracking YACC=false
$MAKE
ls -l sub/*.[ch] && exit 1

env DISTCHECK_CONFIGURE_FLAGS='YACC=false' $MAKE distcheck

:
