#! /bin/sh
# Copyright (C) 2004-2018 Free Software Foundation, Inc.
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

# Test to make sure we diagnose misplaced AC_CONFIG_AUX_DIR.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_AUX_DIR([.]) dnl this will appear after AM_INIT_AUTOMAKE
END

: > Makefile.am

$ACLOCAL
AUTOMAKE_fails
grep 'AC_CONFIG_AUX_DIR.*AM_INIT_AUTOMAKE' stderr

:
