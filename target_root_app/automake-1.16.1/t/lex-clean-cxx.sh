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

# Check that C++ source files derived from non-distributed Lex sources
# are cleaned by "make clean", while C++ source files derived from
# distributed Lex sources are cleaned by "make maintainer-clean".
# See also sister test 'lex-clean.sh'.

required='c++ lex'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CXX
AC_PROG_LEX
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo bar baz qux

foo_SOURCES = mainfoo.cc parsefoo.lxx

bar_SOURCES = mainbar.cpp parsebar.ll
bar_LFLAGS = $(AM_LFLAGS)

baz_SOURCES = mainbaz.c++
nodist_baz_SOURCES = parsebaz.l++

qux_SOURCES = mainqux.cxx
nodist_qux_SOURCES = parsequx.lpp
qux_LFLAGS = $(AM_LFLAGS)

parsebaz.l++ parsequx.lpp:
	cp $(srcdir)/parsefoo.lxx $@

CLEANFILES = parsebaz.l++ parsequx.lpp

LDADD = $(LEXLIB)
END

cat > parsefoo.lxx << 'END'
%{
#define YY_DECL int yylex (void)
extern "C" YY_DECL;
#define YY_NO_UNISTD_H 1
int isatty (int fd) { return 0; }
%}
%%
"GOOD"   return EOF;
.
%%
int yywrap (void)
{
  return 1;
}
END
cp parsefoo.lxx parsebar.ll

cat > mainfoo.cc << 'END'
// This file should contain valid C++ but invalid C.
extern "C" int yylex (void);
using namespace std;
int main (int argc, char **argv)
{
  return yylex ();
}
END
cp mainfoo.cc mainbar.cpp
cp mainfoo.cc mainbaz.c++
cp mainfoo.cc mainqux.cxx

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

cp config.status config.sav

$MAKE
ls -l
# Sanity checks.
test -f parsefoo.cxx
test -f bar-parsebar.cc
test -f parsebaz.l++
test -f parsebaz.c++
test -f parsequx.lpp
test -f qux-parsequx.cpp

for target in clean distclean; do
  $MAKE $target
  ls -l
  test -f parsefoo.cxx
  test -f bar-parsebar.cc
  test ! -e parsebaz.l++
  test ! -e parsebaz.c++
  test ! -e parsequx.lpp
  test ! -e qux-parsequx.cpp
done

cp config.sav config.status
./config.status # re-create Makefile

$MAKE maintainer-clean
ls -l
test -f parsefoo.lxx
test -f parsebar.ll
test ! -e parsefoo.cxx
test ! -e bar-parsebar.cc
test -f parsefoo.lxx
test -f parsebar.ll
test ! -e parsefoo.cxx
test ! -e bar-parsebar.cc

:
