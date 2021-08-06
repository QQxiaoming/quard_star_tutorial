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

# Make sure that installing subdirectory libraries works.
# PR/300

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_RANLIB
AC_PROG_CC
AM_PROG_AR
AC_OUTPUT
END

cat > Makefile.am << 'END'
lib_LIBRARIES = subdir/liba.a
subdir_liba_a_SOURCES = a.c

nobase_lib_LIBRARIES = subdir/libb.a
subdir_libb_a_SOURCES = a.c
END

cat > a.c << 'END'
int i = 3;
END

## A rule in the Makefile should create subdir
# mkdir subdir

$ACLOCAL
$AUTOCONF
$AUTOMAKE --copy --add-missing

# We pass '--libdir' explicitly, to avoid spurious failures due to users
# or distributions possibly overriding '${libdir}' in their $CONFIG_SITE
# file (for example, defining it to '${prefix}/lib64' on 64-bit systems,
# as is the case with openSUSE 12.1).  See automake bug#10426.
cwd=$(pwd) || fatal_ "getting current working directory"
./configure --prefix "$cwd/inst" --libdir "$cwd/inst/lib"

$MAKE

test -f subdir/liba.a
test -f subdir/libb.a

$MAKE install

test -f inst/lib/liba.a
test -f inst/lib/subdir/libb.a

$MAKE uninstall

test -f inst/lib/liba.a && exit 1
test -f inst/lib/subdir/libb.a && exit 1

$MAKE install-strip

test -f inst/lib/liba.a
test -f inst/lib/subdir/libb.a

:
