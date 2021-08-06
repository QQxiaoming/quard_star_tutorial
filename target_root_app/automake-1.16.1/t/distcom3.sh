#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Test to make sure that non-existing common files are distributed
# if they are buildable.

. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am << 'END'
README:
	echo 'I bet you are reading me.' > README
test-distcommon:
	echo ' ' $(DIST_COMMON) ' ' | grep ' README '
END

# Files required by '--gnu'.
: > NEWS
: > AUTHORS
: > ChangeLog

$ACLOCAL

# Should not warn about missing README, since it is a target.
$AUTOMAKE --add-missing --gnu >output 2>&1 || { cat output; exit 1; }
cat output
grep README output && exit 1

$AUTOCONF
./configure
$MAKE test-distcommon
$MAKE distdir
test -f $distdir/README

# Should warn about missing README.
rm -f README
: > Makefile.am
AUTOMAKE_fails --add-missing --gnu
grep 'required file.*README.*not found' stderr

:
