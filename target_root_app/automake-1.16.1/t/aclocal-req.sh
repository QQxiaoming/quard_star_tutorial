#! /bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

# Test to make sure macro used in m4 file is included.

. test-init.sh

cat > configure.ac <<EOF
AC_INIT([$me], [1.0])
AM_INIT_GUILE_MODULE
EOF

cat > acinclude.m4 << 'END'
AC_DEFUN([AM_INIT_GUILE_MODULE], [AM_INIT_AUTOMAKE([whocares])])
END

$ACLOCAL

grep 'DEFUN.*INIT_AUTOMAKE' aclocal.m4

:
