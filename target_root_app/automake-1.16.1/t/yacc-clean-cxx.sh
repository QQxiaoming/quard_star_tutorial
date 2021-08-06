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

# Check that C++ source and header files derived from non-distributed
# Yacc sources are cleaned by "make clean", while C++ source and
# header files derived from distributed Yacc sources are cleaned by
# "make maintainer-clean".
# See also sister test 'yacc-clean.sh'.

required='c++ yacc'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CXX
AC_PROG_YACC
AC_CONFIG_FILES([sub1/Makefile sub2/Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
# Use two subdirectories, one to test with '-d' in YFLAGS, the
# other one to test with empty YFLAGS.
SUBDIRS = sub1 sub2
END

mkdir sub1 sub2

cat > sub1/Makefile.am << 'END'
bin_PROGRAMS = foo bar baz qux

foo_SOURCES = mainfoo.cc parsefoo.yxx

bar_SOURCES = mainbar.cpp parsebar.yy
bar_YFLAGS = $(AM_YFLAGS)

baz_SOURCES = mainbaz.c++
nodist_baz_SOURCES = parsebaz.y++

qux_SOURCES = mainqux.cxx
nodist_qux_SOURCES = parsequx.ypp
qux_YFLAGS = $(AM_YFLAGS)

parsebaz.y++ parsequx.ypp:
	cp $(srcdir)/parsefoo.yxx $@

CLEANFILES = parsebaz.y++ parsequx.ypp
END

cat > sub2/Makefile.am << 'END'
include $(top_srcdir)/sub1/Makefile.am
AM_YFLAGS = -d
END

cat > sub1/parsefoo.yxx << 'END'
%{
// This file should contain valid C++ but invalid C.
#include <cstdio>
// "std::" qualification required by Sun C++ 5.9.
int yylex (void) { return std::getchar (); }
void yyerror (const char *s) { return; }
%}
%%
x : 'x' { };
END
cp sub1/parsefoo.yxx sub1/parsebar.yy
cp sub1/parsefoo.yxx sub2/parsefoo.yxx
cp sub1/parsefoo.yxx sub2/parsebar.yy

cat > sub1/mainfoo.cc << 'END'
// This file should contain valid C++ but invalid C.
using namespace std;
int main (int argc, char **argv)
{
  extern int yyparse (void);
  return yyparse ();
}
END
cp sub1/mainfoo.cc sub1/mainbar.cpp
cp sub1/mainfoo.cc sub1/mainbaz.c++
cp sub1/mainfoo.cc sub1/mainqux.cxx
cp sub1/main???.c* sub2

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

cp config.status config.sav

$MAKE
ls -l . sub1 sub2
# Sanity checks.
test -f sub1/parsefoo.cxx
test -f sub1/bar-parsebar.cc
test -f sub1/parsebaz.y++
test -f sub1/parsebaz.c++
test -f sub1/parsequx.ypp
test -f sub1/qux-parsequx.cpp
test -f sub2/parsefoo.cxx
test -f sub2/parsefoo.hxx
test -f sub2/bar-parsebar.cc
test -f sub2/bar-parsebar.hh
test -f sub2/parsebaz.y++
test -f sub2/parsebaz.c++
test -f sub2/parsebaz.h++
test -f sub2/parsequx.ypp
test -f sub2/qux-parsequx.cpp
test -f sub2/qux-parsequx.hpp

for target in clean distclean; do
  $MAKE $target
  ls -l . sub1 sub2
  test -f sub1/parsefoo.cxx
  test -f sub1/bar-parsebar.cc
  test ! -e sub1/parsebaz.y++
  test ! -e sub1/parsebaz.c++
  test ! -e sub1/parsequx.ypp
  test ! -e sub1/qux-parsequx.cpp
  test -f sub2/parsefoo.cxx
  test -f sub2/parsefoo.hxx
  test -f sub2/bar-parsebar.cc
  test -f sub2/bar-parsebar.hh
  test ! -e sub2/parsebaz.y++
  test ! -e sub2/parsebaz.c++
  test ! -e sub2/parsebaz.h++
  test ! -e sub2/parsequx.ypp
  test ! -e sub2/qux-parsequx.cpp
  test ! -e sub2/qux-parsequx.hpp
done

cp config.sav config.status
./config.status # re-create Makefile

$MAKE maintainer-clean
ls -l . sub1 sub2
test -f sub1/parsefoo.yxx
test -f sub1/parsebar.yy
test ! -e sub1/parsefoo.cxx
test ! -e sub1/bar-parsebar.cc
test -f sub2/parsefoo.yxx
test -f sub2/parsebar.yy
test ! -e sub2/parsefoo.cxx
test ! -e sub2/parsefoo.hxx
test ! -e sub2/bar-parsebar.cc
test ! -e sub2/bar-parsebar.hh

:
