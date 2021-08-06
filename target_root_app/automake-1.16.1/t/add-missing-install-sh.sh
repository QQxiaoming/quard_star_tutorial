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

# Test for this bug:
# automake: Makefile.am: required file "../../install-sh" not found; installing
# This also makes sure that install-sh is created in the correct directory.

. test-init.sh

: > Makefile.am
rm -f install-sh

# Since the default path includes '../..', we must run this test in
# yet another subdir.
mkdir frob
mv Makefile.am configure.ac frob/
cd frob

$ACLOCAL
$AUTOMAKE --add-missing >output 2>&1 || { cat output; exit 1; }
cat output

# Only one '/' should appear in the output.
grep '/.*/' output && exit 1

test -f install-sh

:
