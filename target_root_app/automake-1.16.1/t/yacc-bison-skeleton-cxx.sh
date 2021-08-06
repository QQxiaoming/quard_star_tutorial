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

# Test to make sure bison + bison's C++ skeleton + C++ works.
# For Automake bug#7648 and PR automake/491.

required='c++ bison'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CXX
AC_PROG_YACC
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = zardoz
zardoz_SOURCES = zardoz.yy foo.cc

# This is required even with %defines in zardoz.yy.
AM_YFLAGS = -d

BUILT_SOURCES = zardoz.hh
EXTRA_DIST = stack.hh location.hh position.hh
END

cat > zardoz.yy << 'END'
%skeleton "lalr1.cc"
%defines
%locations

%union
{
  int ival;
};
%{
int yylex (yy::parser::semantic_type *yylval,
           yy::parser::location_type *yylloc);
%}

%%
start :        /* empty */
%%

int
yylex (yy::parser::semantic_type *yylval,
       yy::parser::location_type *yylloc)
{
  return 0;
}

void
yy::parser::error(const yy::parser::location_type&, const std::string&)
{
  return;
}
END

cat > foo.cc << 'END'
#include "zardoz.hh"

int
main(int argc, char** argv)
{
  yy::parser my_parser;
  return my_parser.parse ();
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

# Try a VPATH build first.
mkdir build
cd build
../configure YACC='bison -y'
$MAKE
cd ..

# Now try an in-tree build.
./configure YACC='bison -y'
$MAKE

# Check that distribution is self-contained, and do not require
# bison to be built.
yl_distcheck YACC=false DISTCHECK_CONFIGURE_FLAGS='YACC=false'

:
