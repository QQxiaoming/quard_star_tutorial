#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Make sure aclocal define macros in the same order as -I's.

am_create_testdir=empty
. test-init.sh

cat > configure.ac << 'END'
AC_INIT
MACRO1
MACRO2
END

mkdir m4_1 m4_2

cat >m4_1/somedefs.m4 <<EOF
AC_DEFUN([MACRO1], [:macro11:])
AC_DEFUN([MACRO2], [:macro21:])
EOF

cat >m4_2/somedefs.m4 <<EOF
AC_DEFUN([MACRO1], [:macro12:])
EOF

$ACLOCAL -I m4_1 -I m4_2
$AUTOCONF
$FGREP ':macro11:' configure
$FGREP ':macro21:' configure

$ACLOCAL --force -I m4_2 -I m4_1
$AUTOCONF --force
$FGREP ':macro12:' configure
$FGREP ':macro21:' configure

:
