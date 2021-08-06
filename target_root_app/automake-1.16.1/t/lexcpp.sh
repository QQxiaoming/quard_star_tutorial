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

# Test to make sure Lex + C++ is supported.
# Please keep this is sync with sister test 'yaccpp.sh'.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CXX
AC_PROG_LEX
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo bar baz qux
foo_SOURCES = foo.l++
bar_SOURCES = bar.lpp
baz_SOURCES = baz.ll
qux_SOURCES = qux.lxx
END

$ACLOCAL
$AUTOMAKE -a

sed -e 's/^/ /' -e 's/$/ /' Makefile.in >mk
$FGREP ' foo.c++ ' mk
$FGREP ' bar.cpp ' mk
$FGREP ' baz.cc '  mk
$FGREP ' qux.cxx ' mk

:
