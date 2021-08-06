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

# Check that inclusion of '.am' fragments by automake does not suffer
# of the "deleted header problem".  This test checks that we can rename
# an included .am file without confusing the remake rules.

. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am <<'END'
include foo.am
END

cat > foo.am <<'END'
all-local:
	echo ok > has-run-1
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure
$MAKE
test -f has-run-1

$sleep
rm -f has-run-1
mv -f foo.am bar.am
echo include bar.am > Makefile.am
$MAKE Makefile
$FGREP 'foo.am' Makefile Makefile.in && exit 1

$sleep
rm -f has-run-1 bar.am
echo 'all-local:; echo ok > has-run-2' > zardoz.am
echo 'include $(srcdir)/zardoz.am' > Makefile.am
using_gmake || $MAKE Makefile
$MAKE
test ! -e has-run-1
test -f has-run-2

:
