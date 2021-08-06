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

# Test of subdir objects with C++.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CXX
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = wish
wish_SOURCES = generic/a.cc generic/b.cxx
END

$ACLOCAL
$AUTOMAKE

$FGREP 'generic/a.$(OBJEXT)' Makefile.in
grep '[^/]a\.\$(OBJEXT)' Makefile.in && exit 1
grep '.*-c -o' Makefile.in

:
