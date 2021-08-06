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

# Test to make sure misspellings in _SOURCES variables cause failure.

. test-init.sh

echo AC_PROG_CC >> configure.ac

cat > Makefile.am << 'END'
bin_PROGRAMS = zardoz
zardoz_SOURCES = x.c
qardoz_LDADD = -ljoe
END

$ACLOCAL
AUTOMAKE_fails
grep 'Makefile.am:3:.*qardoz' stderr
