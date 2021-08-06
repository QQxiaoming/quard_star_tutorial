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

# Non-existent directories in ACLOCAL_PATH are ok.

am_create_testdir=empty
. test-init.sh

cat > configure.ac << 'END'
AC_INIT
AM_FOO
END

mkdir mdir
echo 'AC_DEFUN([AM_FOO], [am--foo])' > mdir/foo.m4

ACLOCAL_PATH=./nonesuch:./mdir:$(pwd)/nonesuch2:nonesuch3 $ACLOCAL
$AUTOCONF
$FGREP 'am--foo' configure

:
