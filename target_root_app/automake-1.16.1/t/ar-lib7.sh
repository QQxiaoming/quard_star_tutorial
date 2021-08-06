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

# Test if automake warns if ar-lib is missing when AM_PROG_AR is used.

. test-init.sh

cat >> configure.ac << 'END'
AM_PROG_AR
END

:> Makefile.am

$ACLOCAL
AUTOMAKE_fails

grep '^configure\.ac:.*ar-lib.*not found' stderr

$AUTOMAKE --add-missing

:
