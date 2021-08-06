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

# Use of JAVA primary should not allow the use of the 'java_' prefix
# when $(javadir) is undefined.  Otherwise, we could silently end up
# with a broken Makefile.in (where "make install" doesn't install
# the *.class files).
# See automake bug#8461.

. test-init.sh

$ACLOCAL

echo java_JAVA = a.java > Makefile.am
AUTOMAKE_fails
grep '^Makefile\.am:1:.*java_JAVA.*javadir.* undefined' stderr
$EGREP '(uninitialized|line) ' stderr && exit 1

echo javadir = a-dummy-value >> Makefile.am
$AUTOMAKE

:
