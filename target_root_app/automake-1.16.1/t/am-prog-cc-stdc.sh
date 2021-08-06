#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Check that the obsolete macro the obsolete macro AM_PROG_CC_STDC
# still works.

required=gcc
. test-init.sh

cat >> configure.ac <<'END'
AM_PROG_CC_STDC
AC_OUTPUT
END

echo bin_PROGRAMS = foo > Makefile.am

$ACLOCAL
$AUTOMAKE

$AUTOCONF -Wnone -Wobsolete -Werror 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
grep "^configure\.ac:4:.*'AM_PROG_CC_STDC'.*obsolete" stderr
grep "'AC_PROG_CC'.* instead" stderr

echo 'int main (void) { return 0; }' > foo.c

./configure
$MAKE
$MAKE distcheck

:
