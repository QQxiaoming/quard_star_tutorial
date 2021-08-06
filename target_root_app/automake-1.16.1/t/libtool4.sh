#!/bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Make sure Automake suggests AC_PROG_LIBTOOL when *_LTLIBRARIES is used.

. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AM_PROG_AR
END

cat > Makefile.am << 'END'
EXTRA_LTLIBRARIES = liblib.la
END

: > ar-lib

$ACLOCAL
AUTOMAKE_fails
grep '[Ll]ibtool library .*LIBTOOL.* undefined' stderr
grep 'define .*LIBTOOL.* add .*LT_INIT' stderr

:
