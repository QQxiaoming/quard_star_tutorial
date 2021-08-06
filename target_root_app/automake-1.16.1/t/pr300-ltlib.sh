#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Make sure that installing subdirectory libtool libraries works.
# PR/300

required='cc libtoolize'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_LIBTOOL
AC_OUTPUT
END

cat > Makefile.am << 'END'
lib_LTLIBRARIES = subdir/liba.la
subdir_liba_la_SOURCES = a.c

nobase_lib_LTLIBRARIES = subdir/libb.la
subdir_libb_la_SOURCES = a.c
END

cat > a.c << 'END'
int i = 3;
END

libtoolize
$ACLOCAL
$AUTOCONF
$AUTOMAKE --copy --add-missing

# We pass '--libdir' explicitly, to avoid spurious failures due to users
# or distributions possibly overriding '${libdir}' in their $CONFIG_SITE
# file (for example, defining it to '${prefix}/lib64' on 64-bit systems,
# as is the case with openSUSE 12.1).  See automake bug#10426.
cwd=$(pwd) || fatal_ "getting current working directory"
./configure --prefix "$cwd/inst" --libdir "$cwd/inst/lib"

# A rule in the Makefile should create subdir.
test ! -e subdir
run_make -O
test -d subdir

grep 'liba.la .*-rpath .*lib' stdout
grep 'liba.la .*-rpath .*lib/subdir' stdout && exit 1
grep 'libb.la .*-rpath .*lib/subdir' stdout

test -f subdir/liba.la
test -f subdir/libb.la

run_make -E install
grep 'remember.*--finish' stderr && exit 1

test -f inst/lib/liba.la
test -f inst/lib/subdir/libb.la

$MAKE uninstall

test -f inst/lib/liba.la && exit 1
test -f inst/lib/subdir/libb.la && exit 1

$MAKE install-strip

test -f inst/lib/liba.la
test -f inst/lib/subdir/libb.la

:
