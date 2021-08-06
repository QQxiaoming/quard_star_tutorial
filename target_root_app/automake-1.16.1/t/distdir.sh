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

# Test to make sure subdirs in EXTRA_DIST work.  Also tests to make
# sure *srcdir is properly handled.  Note that using './', as in
#   EXTRA_DIST = ./joe
# does not work portably: it fails with HP-UX and Tru64 make.
# Also test DISTFILES containing a directory and a file in it,
# and repeated directories.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
include_HEADERS = some/file another/sub/subsub/file2 yet/another/file3
EXTRA_DIST = foo/bar joe $(top_srcdir)/woo/doo $(srcdir)/dada \
	     some another/sub yet \
	     some another/sub yet


all-local:
	$(MKDIR_P) another/sub/subsub
	touch another/sub/subsub/file2

CLEANFILES = another/sub/subsub/file2

check-local:
	test -f $(srcdir)/foo/bar
	test -f $(srcdir)/woo/doo
	test -f $(srcdir)/joe
	test -f $(srcdir)/dada
	test -f $(srcdir)/some/file
	test -f $(srcdir)/another/sub/subsub/file2 \
	|| test -f /another/sub/subsub/file2
	test -f $(srcdir)/yet/another/file3
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

mkdir foo woo some another another/sub another/sub/subsub yet yet/another
touch foo/bar joe woo/doo dada some/file another/sub/subsub/file2
touch yet/another/file3

mkdir build
cd build
../configure
$MAKE distdir
# Check to make sure 'foo' isn't made in build directory.
test -d foo && exit 1

rm -rf $me-1.0
# Remove the dot from VERSION for the next grep.
run_make -O -e IGNORE VERSION=10 MKDIR_P='echo MKDIR_P' distdir

# Make sure no './' appear in the directory names.  srcdir is '..', so
# this also checks that no directory is created in the source tree.
grep 'MKDIR_P.*\.' stdout && exit 1

cd ..
./configure --prefix "$(pwd)"
$MAKE distcheck

:
