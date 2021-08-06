#! /bin/sh
# Copyright (C) 2008-2018 Free Software Foundation, Inc.
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

# Unsafe absolute directory names should be diagnosed.

. test-init.sh

mkdir 'unsafe$'
cd 'unsafe$'

mv ../configure.ac .
mv ../install-sh ../missing .

: > Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
grep 'unsafe absolute working directory' stderr

cd ..
mkdir build
cd build
../unsafe$/configure 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
grep 'unsafe srcdir' stderr

:
