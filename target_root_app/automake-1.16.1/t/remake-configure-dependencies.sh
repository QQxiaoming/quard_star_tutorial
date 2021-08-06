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

# Test CONFIGURE_DEPENDENCIES.

. test-init.sh

cat > configure.ac <<'END'
AC_INIT([confdeps], m4_esyscmd([./print-version]))
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

# Needless to say, the autom4te cache happily break this test.
cat > .autom4te.cfg <<'END'
begin-language: "Autoconf-without-aclocal-m4"
args: --no-cache
end-language: "Autoconf-without-aclocal-m4"
END

cat > Makefile.am <<'END'
EXTRA_DIST = .autom4te.cfg
CONFIGURE_DEPENDENCIES = $(top_srcdir)/print-version

test-1:
	test $(VERSION) = 1.0
test-2:
	test $(VERSION) = 2.1
test-3:
	test $(VERSION) = 3.14

.PHONY: test-1 test-2 test-3

# Contents of $(CONFIGURE_DEPENDENCIES) are to be automatically
# distributed.
local-check: test-3 distdir
	test -f $(distdir)/print-version
END

(echo '#!/bin/sh' && echo 'printf %s 1.0') > print-version
chmod a+x print-version

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE test-1

$sleep
(echo '#!/bin/sh' && echo 'printf %s 2.1') > print-version
using_gmake || $MAKE Makefile
$MAKE test-2

$MAKE distclean
mkdir build
cd build
../configure
$MAKE test-2

$sleep
(echo '#!/bin/sh' && echo 'printf %s 3.14') > ../print-version
using_gmake || $MAKE Makefile
$MAKE test-3

# Check that $(CONFIGURE_DEPENDENCIES) are automatically distributed.
$MAKE distcheck

:
