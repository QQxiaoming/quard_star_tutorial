#! /bin/sh
# Copyright (C) 2006-2018 Free Software Foundation, Inc.
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

# See how well the rebuild rule handles an aclocal.m4 that was
# generated for another version of autoconf.

. test-init.sh

echo AC_OUTPUT >>configure.ac

touch Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure
$MAKE

sed '1,20 s/m4_defn(\[AC_AUTOCONF_VERSION\]),/9999,/' < aclocal.m4 > aclocal.tmp
cmp aclocal.m4 aclocal.tmp && exit 1

mv aclocal.tmp aclocal.m4

run_make -E
grep 'You have another version of autoconf' stderr
grep 'aclocal.m4:.*this file was generated for' stderr

run_make -E
grep 'You have another version of autoconf' stderr && exit 1
grep 'aclocal.m4:.*this file was generated for' stderr && exit 1

:
