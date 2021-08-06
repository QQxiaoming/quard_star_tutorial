#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Test to make sure that noinst_* and check_* are not installed.
# From Pavel Roskin.

required=cc
. test-init.sh

cat > Makefile.am << 'END'
noinst_SCRIPTS = foo.sh
noinst_DATA = foo.xpm
noinst_LIBRARIES = libfoo.a
noinst_PROGRAMS = foo
noinst_HEADERS = foo.h
check_SCRIPTS = bar.sh
check_DATA = bar.xpm
check_LIBRARIES = libbar.a
check_PROGRAMS = bar
check_HEADERS = bar.h
END

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_OUTPUT
END

: > ar-lib

$ACLOCAL
$AUTOMAKE

$EGREP '(noinst|check)dir' Makefile.in && exit 1

$AUTOCONF
./configure --prefix="$(pwd)/inst"

echo 'int main (void) { return 0; }' > foo.c
echo 'int main (void) { return 0; }' > bar.c

echo 'int foo (void) { return 0; }' > libfoo.c
echo 'int bar (void) { return 0; }' > libbar.c

: > foo.sh
: > foo.xpm
: > foo.h
: > bar.sh
: > bar.xpm
: > bar.h

$MAKE
$MAKE install
test ! -e inst

:
