#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Demo on C++ support.

required=c++
am_create_testdir=empty
. test-init.sh

cat > configure.ac << 'END'
AC_INIT([GNU C++ Demo], [1.3], [bug-automake@gnu.org])
AC_CONFIG_SRCDIR([play.c++])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE
# The C compiler shouldn't be required in any way.
CC=false; AC_SUBST([CC])
AC_PROG_CXX
AH_BOTTOM([
#ifndef GREETINGS
#  define GREETINGS "Howdy"
#endif])
AC_DEFINE([OK_AC], [1],
          [Give "good to go" declaration from configure.ac])
AC_CONFIG_HEADERS([config.hxx])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = work play
common_sources = common.hpp foo.cpp sub/bar.cc
AM_CPPFLAGS = -DOK_AM=1
play_SOURCES = play.c++ play.hh $(common_sources)
work_SOURCES = work.cxx work.h++ $(common_sources)
work_CXXFLAGS = -D'GREETINGS="Good morning"'

.PHONY: test-objs
check-local: test-objs
test-objs:
	test -f play.$(OBJEXT)
	test -f foo.$(OBJEXT)
	test -f sub/bar.$(OBJEXT)
	test -f work-foo.$(OBJEXT)
	test -f sub/work-bar.$(OBJEXT)
	test -f work-work.$(OBJEXT)
END

mkdir sub build-aux

$ACLOCAL
$AUTOHEADER
test -f config.hxx.in
$AUTOCONF
$AUTOMAKE --add-missing
test -f build-aux/depcomp
# Not needed by C++ compilers.
test ! -e build-aux/compile

cat > work.h++ << 'END'
#define ACTION "work"
class Hello_CXX
{
  public:
    Hello_CXX() { }
    virtual ~Hello_CXX () { }
    void hello_cxx_class ();
};
END

cat > play.hh << 'END'
#define ACTION "play"
void hello_cxx_function (void);
END

cat > common.hpp << 'END'
/* Common header. */

#include <config.hxx>

#if !OK_AM
#error "missing OK from Makefile.am"
choke me
#endif

#if !OK_AC
#error "missing OK from configure.ac"
choke me
#endif

#include <iostream>
END

cat > work.cxx << 'END'
#include "common.hpp"
#include "work.h++"
#include <cstdlib>
using namespace std;
int main (void)
{
  cout << "We are working :-(" << endl;
  Hello_CXX *hello = new Hello_CXX;
  hello->hello_cxx_class ();
  return EXIT_SUCCESS;
}
END

cat > play.c++ << 'END'
#include "common.hpp"
#include "play.hh"
int main (void)
{
  std::cout << "We are playing :-)" << std::endl;
  hello_cxx_function ();
  return 0;
}
END

cat > foo.cpp <<'END'
#include <config.hxx>
#include "work.h++"
#include <iostream>
using namespace std;
void Hello_CXX::hello_cxx_class (void)
{
  cout << GREETINGS << ", " << ACTION << "." << endl;
}
END

cat > sub/bar.cc << 'END'
#include <config.hxx>
#include "play.hh"
#include <stdio.h>
void hello_cxx_function (void)
{
  printf ("%s, %s!\n", GREETINGS, ACTION);
}
END

./configure
$MAKE
$MAKE test-objs

if ! cross_compiling; then
  unindent > exp.play << 'END'
    We are playing :-)
    Howdy, play!
END
  unindent > exp.work << 'END'
    We are working :-(
    Good morning, work.
END
  for p in play work; do
    # The program must run correctly (exit status = 0).
    ./$p
    # And it must have the expected output.  Note that we strip extra
    # CR characters (if any), to cater to MinGW programs on MSYS.
    # See automake bug#14493.
    ./$p | tr -d '\015' > got.$p || { cat got.$p; exit 1; }
    cat exp.$p
    cat got.$p
    diff exp.$p got.$p
  done
fi

$MAKE distcheck

:
