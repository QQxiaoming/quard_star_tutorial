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

# Ensure install-strip works when STRIP consists of more than one word.
# This test needs GNU binutils strip.  See sister test 'strip3.sh'.

required='cc strip'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo
lib_LIBRARIES = libfoo.a
END

echo 'int main (void) { return 0; }' > foo.c
echo 'int foo (void) { return 0; }' > libfoo.c

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure --prefix="$(pwd)/inst" STRIP='strip --verbose'
$MAKE
$MAKE install-strip

:
