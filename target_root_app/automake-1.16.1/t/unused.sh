#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Test for failing check for unused macros.  From Johan Danielsson.

. test-init.sh

cat >> configure.ac << 'END'
MACRO_1
MACRO_1_2
MACRO_1_2_3
END

cat > acinclude.m4 << 'END'
AC_DEFUN([MACRO_1], echo 1)
AC_DEFUN([MACRO_1_2], echo 12)
AC_DEFUN([MACRO_1_2_3], echo 123)
END

$ACLOCAL 2>stderr || { cat stderr >&2; exit 1; }
cat stderr >&2
test ! -s stderr
