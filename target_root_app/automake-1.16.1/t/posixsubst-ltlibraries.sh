#! /bin/sh
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

# Test that POSIX variable expansion '$(var:str=rpl)' works when used
# with the LTLIBRARIES primary in a "simple" way.
# Keep this in sync with sister test 'posixsubst-libraries.sh'.

required='cc libtool libtoolize'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_LIBTOOL
AC_OUTPUT
END

cat > Makefile.am << 'END'
foolibs = libfoo1 libfoo2
barlibs = libbaz

# Also try an empty match suffix, to ensure that the ':=' in there is
# not confused by the parser with an unportable assignment operator.
lib_LTLIBRARIES = $(foolibs:=.la) $(barlibs:z=r.la)

libbar.c:
	echo 'int bar(void) { return 0; }' > $@
CLEANFILES = libbar.c # For FreeBSD make.

installcheck-local:
	ls -l $(libdir)
	test -f $(libdir)/libfoo1.la
	test -f $(libdir)/libfoo2.la
	test -f $(libdir)/libbar.la
END

echo 'int bar1(void) { return 0; }' > libfoo1.c
echo 'int bar2(void) { return 0; }' > libfoo2.c

libtoolize
$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

cwd=$(pwd) || fatal_ "getting current working directory"
./configure --prefix="$cwd/_inst"
$MAKE
test -f libfoo2.c
$MAKE install
$MAKE installcheck
$MAKE distcheck

:
