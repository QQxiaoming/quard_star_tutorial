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

# Test to make sure dependencies for Lex and C/C++ does not break
# in obvious ways.  See PR automake/6, and related semantic tests
# 'lex-depend.sh' and 'lex-depend-cxx.sh'.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_CXX
AM_PROG_LEX
END

cat > Makefile.am << 'END'
bin_PROGRAMS = zoo foo
zoo_SOURCES = joe.ll
foo_SOURCES = moe.l
noinst_PROGRAMS = zardoz
zardoz_SOURCES = _0.l _1.ll _2.lxx _3.l++ _4.lpp
END

$ACLOCAL
$AUTOMAKE -a

$EGREP '([mj]oe|_[01234]|include|\.P)' Makefile.in # For debugging.

for x in joe moe _0 _1 _2 _3 _4; do
  $EGREP '\$\(DEPDIR\)/'"$x"'\.Po( |$)' Makefile.in
done

:
