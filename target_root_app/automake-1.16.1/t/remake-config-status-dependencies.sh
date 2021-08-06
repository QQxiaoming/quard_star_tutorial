#! /bin/sh
# Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

# Test CONFIG_STATUS_DEPENDENCIES.

. test-init.sh

cat >> configure.ac <<'END'
. ${srcdir}/version.sh
AC_SUBST([EXTRA_VERSION])
AC_SUBST([CONFIG_STATUS_DEPENDENCIES], ['$(top_srcdir)/version.sh'])
AC_OUTPUT
END

echo EXTRA_VERSION=1.0 > version.sh

cat > Makefile.am <<'END'
.PHONY: test-1 test-2 test-3
check-local: test-3
test-1:
	test $(EXTRA_VERSION) = 1.0
test-2:
	test $(EXTRA_VERSION) = 2.1
test-3:
	test $(EXTRA_VERSION) = 3.14
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE test-1

$sleep
echo EXTRA_VERSION=2.1 > version.sh
using_gmake || $MAKE Makefile
$MAKE test-2

$MAKE distclean
mkdir build
cd build
../configure
$MAKE test-2
$sleep
echo EXTRA_VERSION=3.14 > ../version.sh
using_gmake || $MAKE Makefile
$MAKE test-3

# Contents of $(CONFIG_STATUS_DEPENDENCIES) are *not* to be
# automatically distributed; there is no generic good reason
# for that.
$MAKE distdir
test ! -e $distdir/version.sh
test -f $distdir/configure || exit 99 # Sanity check.

:
