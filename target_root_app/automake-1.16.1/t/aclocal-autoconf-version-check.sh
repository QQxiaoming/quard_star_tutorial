#! /bin/sh
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

# Make sure autoconf version checks in aclocal.m4 are robust.

am_create_testdirs=empty
. test-init.sh

cat > configure.ac <<END
m4_define([AC_AUTOCONF_VERSION], [9999a])
dnl!! m4_define([a], [oops])
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

: > Makefile.am

$ACLOCAL
$AUTOCONF 2>stderr || { cat stderr >&2; exit 1; }
cat stderr >&2
grep 'You have another version of autoconf' stderr
grep 'aclocal.m4:.*this file was generated for' stderr
$AUTOMAKE -a
./configure
$MAKE

sed 's/^dnl!! //' < configure.ac > configure.tmp
cmp configure.ac configure.tmp && fatal_ 'failed to edit configure.ac'
mv -f configure.tmp configure.ac

run_make -E
grep 'You have another version of autoconf' stderr
grep 'aclocal.m4:.*this file was generated for autoconf 9999a' stderr

:
