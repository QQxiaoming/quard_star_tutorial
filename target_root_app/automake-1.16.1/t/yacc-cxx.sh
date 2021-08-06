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

# Basic semantic checks on Yacc + C++ support (when yacc-generated
# headers are not involved).
# Keep in sync with sister test 'yacc-basic.sh'.

required='c++ yacc'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CXX
AC_PROG_YACC
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo1 foo2 foo3 foo4
foo1_SOURCES = parse1.yy  foo.cc
foo2_SOURCES = parse2.y++ bar.c++
foo3_SOURCES = parse3.yxx foo.cc
foo4_SOURCES = parse4.ypp bar2.cxx
foo3_YFLAGS = -v
foo4_YFLAGS = $(foo3_YFLAGS)

.PHONY: echo-distcom
echo-distcom:
	@echo ' ' $(DIST_COMMON) ' '
END

cat > parse1.yy << 'END'
%{
// Valid C++, but deliberately invalid C.
#include <cstdio>
#include <cstdlib>
// "std::" qualification required by Sun C++ 5.9.
int yylex (void) { return std::getchar (); }
void yyerror (const char *s) { return; }
%}
%%
a : 'a' { exit(0); };
END
cp parse1.yy parse2.y++
cp parse1.yy parse3.yxx
cp parse1.yy parse4.ypp

cat > foo.cc << 'END'
// Valid C++, but deliberately invalid C.
using namespace std;
int main (int argc, char **argv)
{
  int yyparse (void);
  yyparse ();
  return 1;
}
END
cp foo.cc bar.c++
cp foo.cc bar2.cxx

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

$MAKE

# The Yacc-derived C++ sources must be created, and not removed once
# compiled (i.e., not treated like "intermediate files" in the GNU
# make sense).
test -f parse1.cc
test -f parse2.c++
test -f foo3-parse3.cxx
test -f foo4-parse4.cpp
# Check that per-object flags are honored.
test -f foo3-parse3.output
test -f foo4-parse4.output

if ! cross_compiling; then
  for i in 1 2 3 4; do
    echo a | ./foo$i
    echo b | ./foo$i && exit 1
    : For shells with busted 'set -e'.
  done
fi

# The Yacc-derived C++ sources must be shipped.
$MAKE echo-distcom
$MAKE -s echo-distcom | grep '[ /]parse1\.cc '
$MAKE -s echo-distcom | grep '[ /]parse2\.c++ '
$MAKE -s echo-distcom | grep '[ /]foo3-parse3\.cxx '
$MAKE -s echo-distcom | grep '[ /]foo4-parse4\.cpp '
$MAKE distdir
ls -l $distdir
test -f $distdir/parse1.cc
test -f $distdir/parse2.c++
test -f $distdir/foo3-parse3.cxx
test -f $distdir/foo4-parse4.cpp

# Sanity check on distribution.
# Note that, for this to succeed, foo3-parse3.output and foo4-parse4.output
# must either not be distributed, or properly cleaned by automake-generated
# rules.  We don't want to set the exact semantics yet, but want to ensure
# they are are consistent.
yl_distcheck

# Make sure that the Yacc-derived C++ sources are erased by
# maintainer-clean, and not by distclean.
test -f parse1.cc
test -f parse2.c++
test -f foo3-parse3.cxx
test -f foo4-parse4.cpp
$MAKE distclean
ls -l
test -f parse1.cc
test -f parse2.c++
test -f foo3-parse3.cxx
test -f foo4-parse4.cpp
./configure # Re-create 'Makefile'.
$MAKE maintainer-clean
ls -l
test ! -e parse1.cc
test ! -e parse2.c++
test ! -e foo3-parse3.cxx
test ! -e foo4-parse4.cpp

:
