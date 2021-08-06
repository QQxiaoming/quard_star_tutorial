#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Make sure we do not distribute header sources when they are built.

. test-init.sh

cat >> configure.ac << 'END'
AC_SUBST([FOO], [NameToBeGrepped])
AC_CONFIG_FILES([include/config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_HEADERS([include/config.h])
AC_OUTPUT
END

cat > Makefile.am << 'END'
.PHONY: test
test: distdir
	test -f $(distdir)/config.h.in
	test -f $(distdir)/include/config.h.in.in
	test ! -f $(distdir)/include/config.h.in
	test ! -r $(distdir)/include/config.h.in
END

mkdir include
: > include/config.h.in.in

$ACLOCAL
$AUTOCONF
$AUTOHEADER
$AUTOMAKE

./configure
$MAKE

$sleep
echo '#undef @FOO@' > include/config.h.in.in
$MAKE include/config.h
# Don't try to be too strict in this grepping, since the substitution
# is done by config.status, and we don't have too much control on it.
grep '/\*.*#undef.*NameToBeGrepped' include/config.h
$MAKE test
$MAKE distcheck

:
