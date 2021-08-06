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

# Test for bug where install-sh not included in distribution.

. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am << 'END'
pkgdata_DATA =
.PHONY: test
test: distdir
	find $(distdir) ;: For debugging.
	echo ' ' $(DISTFILES) ' ' | grep '[ /]install-sh '
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]install-sh '
	test -f $(distdir)/install-sh
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure
$MAKE test

:
