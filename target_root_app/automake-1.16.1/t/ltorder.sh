#!/bin/sh
# Copyright (C) 2008-2018 Free Software Foundation, Inc.
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

# Test for correct installation order of nobase libtool libraries.

required='cc libtoolize'
. test-init.sh

cat >>configure.ac <<'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_LIBTOOL
AC_OUTPUT
END

cat >Makefile.am <<'END'
AUTOMAKE_OPTIONS = subdir-objects
nobase_lib_LTLIBRARIES = liba1.la sub/liba2.la sub/liba3.la liba4.la liba5.la
sub_liba2_la_LIBADD = liba1.la
sub_liba3_la_LIBADD = sub/liba2.la
liba4_la_LIBADD = sub/liba3.la
liba5_la_LIBADD = liba4.la
bin_PROGRAMS = p
p_LDADD = liba5.la
END

mkdir sub
echo 'int a1 () { return 1; }' >liba1.c
echo 'extern int a1 (); int a2 () { return a1 (); }' >sub/liba2.c
echo 'extern int a2 (); int a3 () { return a2 (); }' >sub/liba3.c
echo 'extern int a3 (); int a4 () { return a3 (); }' >liba4.c
echo 'extern int a4 (); int a5 () { return a4 (); }' >liba5.c
echo 'extern int a5 (); int main () { return a5 (); }' >p.c

libtoolize
$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure --prefix="$(pwd)/inst"

# XXX: This is a workaround for a purity mechanism implemented by GNU Guix.
# For more details see
# <https://www.gnu.org/s/guix/manual/html_node/Application-Setup.html#index-ld_002dwrapper>.
GUIX_LD_WRAPPER_ALLOW_IMPURITIES=1
export GUIX_LD_WRAPPER_ALLOW_IMPURITIES

$MAKE
run_make -E install
grep 'has not been installed' stderr && exit 1

$MAKE uninstall
test $(find inst -type f -print | wc -l) -eq 0

:
