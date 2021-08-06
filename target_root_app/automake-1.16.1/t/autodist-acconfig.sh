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

# Check that 'acconfig.h' is automatically distributed if it exists
# (at automake runtime).
# Related to automake bug#7819.

. test-init.sh

cat >> configure.ac <<END
AC_OUTPUT
END

cat > Makefile.am <<'END'
.PHONY: test
test: distdir
	ls -l $(distdir)
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]acconfig\.h '
	test -f $(distdir)/acconfig.h
END

: > acconfig.h

$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure
$MAKE test

:
