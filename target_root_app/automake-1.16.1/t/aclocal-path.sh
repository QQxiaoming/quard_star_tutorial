#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Check basic ACLOCAL_PATH support.

am_create_testdir=empty
. test-init.sh

cat > configure.ac << 'END'
AC_INIT
AM_FOO_MACRO
AM_BAR_MACRO
AM_BAZ_MACRO
END

mkdir mdir1 mdir2 mdir3

cat > mdir1/foo.m4 << 'END'
AC_DEFUN([AM_FOO_MACRO], [am--foo])
END

cat > mdir2/bar.m4 << 'END'
AC_DEFUN([AM_BAR_MACRO], [am--bar])
END

cat > mdir3/baz.m4 << 'END'
AC_DEFUN([AM_BAZ_MACRO], [am--baz])
END

ACLOCAL_PATH=mdir1:./mdir2:$(pwd)/mdir3 $ACLOCAL
$AUTOCONF

# there should be no m4_include in aclocal.m4, even though ACLOCAL_PATH
# contains 'mdir1' and './mdir2' as relative directories.  Only -I
# directories should be subject to file inclusion.
$FGREP m4_include aclocal.m4 && exit 1

$EGREP 'AM_(FOO|BAR|BAZ)_MACRO' configure && exit 1
$FGREP 'am--foo' configure
$FGREP 'am--bar' configure
$FGREP 'am--baz' configure

:
