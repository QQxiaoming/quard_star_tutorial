#! /bin/sh
# Copyright (C) 1997-2018 Free Software Foundation, Inc.
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

# Test to make sure Yacc + C++ is not obviously broken.
# See also related tests 'yacc-cxx.sh' and 'yacc-d-cxx.sh',
# which does much more in-depth checks (but requires an actual
# Yacc program and a working C++ compiler).

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CXX
AC_PROG_YACC
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo bar baz qux
foo_SOURCES = foo.y++
bar_SOURCES = bar.ypp
baz_SOURCES = baz.yy
qux_SOURCES = qux.yxx
END

$ACLOCAL
$AUTOMAKE -a

$EGREP '(\.[ch]|foo|bar|baz|qux)' Makefile.in # For debugging.

$EGREP '(foo|bar|baz|qux)\.h' Makefile.in && exit 1

sed -e 's/^/ /' -e 's/$/ /' Makefile.in >mk

$FGREP ' foo.c++ ' mk
$FGREP ' bar.cpp ' mk
$FGREP ' baz.cc '  mk
$FGREP ' qux.cxx ' mk

cat >> Makefile.am <<END
AM_YFLAGS = -d
qux_YFLAGS = foo
END

$AUTOMAKE

$EGREP '(\.[ch]|foo|bar|baz|qux)' Makefile.in # For debugging.

sed -e 's/^/ /' -e 's/$/ /' Makefile.in >mk

$FGREP ' foo.c++ ' mk
$FGREP ' foo.h++ ' mk
$FGREP ' bar.cpp ' mk
$FGREP ' bar.hpp ' mk
$FGREP ' baz.cc '  mk
$FGREP ' baz.hh '  mk

$EGREP '(^| )foo\.h\+\+(:| .*:)' Makefile.in
$EGREP '(^| )bar\.hpp(:| .*:)'   Makefile.in
$EGREP '(^| )baz\.hh(:| .*:)'    Makefile.in

grep ' foo\.h[ :]' mk && exit 1
grep ' bar\.h[ :]' mk && exit 1
grep ' baz\.h[ :]' mk && exit 1

$FGREP ' qux-qux.cxx ' mk
$EGREP '(^| )qux-qux\.cxx(:| .*:)' Makefile.in
grep 'qux\.h.*:' Makefile.in && exit 1

:
