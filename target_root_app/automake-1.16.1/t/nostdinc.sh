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

# Test to make sure nostdinc option works correctly.

# We don't require a C compiler explicitly, because the first part of the
# test (where 'Makefile.in' is grepped) does not require one.  Insted, we
# just skip the rest of the test if configure fails to find a working C
# compiler.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = nostdinc
bin_PROGRAMS = foo
foo_SOURCES = foo.c
END

$ACLOCAL
$AUTOMAKE

$EGREP '(-I *\.|-I.*srcdir|am__isrc)' Makefile.in && exit 1

# We'll test the fully-processed Makefile too.
$AUTOCONF

# Test with $builddir != $srcdir
mkdir build
cd build
../configure || exit $?
$EGREP '.*-I *(\.|\$.srcdir.)' Makefile && exit 1

# Test with $builddir = $srcdir
cd ..
./configure || exit $?
$EGREP '.*-I *(\.|\$.srcdir.)' Makefile && exit 1

exit 0
