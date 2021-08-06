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

# Check that many different Yacc parsers (both C and C++) can co-exists
# in the same directory.

required='cc c++ yacc'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_CXX
AC_PROG_YACC
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = c1 c2 cxx1 cxx2 cxx3
AM_YFLAGS = -d

c1_SOURCES = p.y p.h 1.c
c2_SOURCES = p.y 2.c
c2_YFLAGS =

cxx1_SOURCES = parse.yy main1.cc parse.hh

cxx2_SOURCES = parse2.y++ main2.c++
cxx2_YFLAGS =

cxx3_SOURCES = parse3.yxx main3.cxx

BUILT_SOURCES = p.h parse.hh parse3.hxx
END

# The content of all the .c and .y files created below is valid C but
# deliberately invalid C++.
# Vice versa, the content of all the .c++, .cxx, .cc, .y++, .yxx and
# .yy files created below is valid C++ but deliberately invalid C.

cat > p.y <<'END'
%{
int yylex (void) { int new = 0; return new; }
void yyerror (char *s) { return; }
%}
%token ZARDOZ
%%
x : 'x' {};
%%
END

cat > 1.c <<'END'
#include "p.h"
int main ()
{
    int new = ZARDOZ;
    return yyparse () + new;
}

END

cat > 2.c <<'END'
int main ()
{
    int yyparse ();
    int new = 0;
    return yyparse () + new;
}
END

cat > parse.yy <<'END'
%{
#include <cstdlib>
#include "parse.hh"
int yylex (void) { return 0; }
void yyerror (const char *s) { return; }
%}
%token FOOBAR
%%
x : 'x' {};
%%
END

cat > parse2.y++ <<'END'
%{
#include <cstdlib>
int yylex (void) { return 0; }
void yyerror (const char *s) { return; }
%}
%%
x : 'x' {};
%%
END

cat > main1.cc <<'END'
using namespace std;
#include "parse.hh"
int main (int argc, char **argv)
{
    int yyparse (void);
    return yyparse () + FOOBAR;
}
END

cat > main2.c++ <<'END'
using namespace std;
int main (int argc, char **argv)
{
    int yyparse (void);
    return yyparse ();
}
END

edit () { sed -e 's/FOOBAR/BAZQUUX/' -e 's/"parse\.hh"/"parse3.hxx"/'; }
edit <parse.yy >parse3.yxx
edit <main1.cc >main3.cxx

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

# Try a VPATH and by default serial build first, and then an in-tree
# and by default parallel build.

for try in 0 1; do

  if test $try -eq 0; then
    # VPATH serial build.
    mkdir build
    cd build
    srcdir=..
    debug_info="ls -l . $srcdir"
    run_make=$MAKE
  elif test $try -eq 1; then
    # In-tree parallel build.
    srcdir=.
    debug_info="ls -l"
    case $MAKE in
      *\ -j*)
        # Degree of parallelism already specified by the user: do
        # not override it.
        run_make=$MAKE;;
      *)
        # Some make implementations (e.g., HP-UX) don't grok '-j',
        # some require no space between '-j' and the number of jobs
        # (e.g., older GNU make versions), and some *do* require a
        # space between '-j' and the number of jobs (e.g., Solaris
        # dmake).  We need a runtime test to see what works.
        echo 'all:' > Makefile
        for run_make in "$MAKE -j3" "$MAKE -j 3" "$MAKE"; do
          $run_make && break
        done
        rm -f Makefile
    esac
  else
    echo "$me: invalid value of \$try '$try'" >&2
    exit 99
  fi

  $srcdir/configure

  $run_make
  $debug_info

  test -f p.c
  test -f p.h
  test -f c2-p.c
  test ! -e c2-p.h

  test -f parse.cc
  test -f parse.hh
  test -f parse3.cxx
  test -f parse3.hxx

  test -f cxx2-parse2.c++
  test ! -e parse2.h++
  test ! -e cxx2-parse2.h++

  # Minimal checks about recovering from header removal.
  rm -f p.h parse.hh parse3.hxx
  $run_make p.h parse.hh
  $debug_info
  test -f p.h
  test -f parse.hh
  test ! -e parse3.hxx
  $run_make
  $debug_info
  test -f parse3.hxx

  cd $srcdir

done

:
