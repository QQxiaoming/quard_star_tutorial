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

# Make sure that when two files define the same macro in the same
# directory, the macro from the lexically greatest file is used.
# Same as aclocal-I-order-2.sh, but without calling MACRO2.

am_create_testdir=empty
. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
MACRO1
END

mkdir m4

cat >m4/version1.m4 <<EOF
AC_DEFUN([MACRO1], [:macro11:])
AC_DEFUN([MACRO2], [:macro21:])
EOF

cat >m4/version2.m4 <<EOF
AC_DEFUN([MACRO1], [:macro12:])
EOF

$ACLOCAL --verbose -I m4
$AUTOCONF
$FGREP ':macro11:' configure && exit 1
$FGREP ':macro21:' configure && exit 1
$FGREP ':macro12:' configure

:
