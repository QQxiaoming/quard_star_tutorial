#!/bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Test install when a conditional install-*-hook is defined.
# Keep this in sync with sister test 'condhook.sh'.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([TEST], [true])
AC_OUTPUT
END

cat > Makefile.am << 'END'
sysconf_DATA = mumble
if TEST
install-data-hook:
	: > $(top_srcdir)/good
endif
END

: > mumble

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure --prefix "$(pwd)/inst"

$MAKE install
test -f inst/etc/mumble
test -f good

:
