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

# Test to make sure links to _identical files_ created by AC_CONFIG_LINKS get
# removed with 'make distclean' only if doing a VPATH build.

. test-init.sh

mkdir nonmk-subdir sdir sdir/mk-subdir
: > src
: > sdir/Makefile.am
: > sdir/src2
: > nonmk-subdir/src3
: > sdir/mk-subdir/src4

cat >> Makefile.am <<'EOF'
SUBDIRS = sdir
.PHONY: test
test: distdir
	test -f $(distdir)/src
	test -f $(distdir)/sdir/src2
	test -f $(distdir)/nonmk-subdir/src3
	test -f $(distdir)/sdir/mk-subdir/src4
EOF

cat >>configure.ac << 'EOF'
AC_CONFIG_FILES([sdir/Makefile])
AC_CONFIG_LINKS([src:src])
AC_CONFIG_LINKS([sdir/src2:sdir/src2])
AC_CONFIG_LINKS([nonmk-subdir/src3:nonmk-subdir/src3])
AC_CONFIG_LINKS([sdir/mk-subdir/src4:sdir/mk-subdir/src4])
AC_OUTPUT
EOF

$ACLOCAL
$AUTOMAKE
$AUTOCONF

mkdir build
cd build
../configure

$MAKE test

# Make sure nothing is deleted by 'make clean'
$MAKE clean

test -f ../src
test -f ../sdir/src2
test -f ../nonmk-subdir/src3
test -f ../sdir/mk-subdir/src4

# Make sure the links are deleted by 'make distclean' and the original files
# are not.
$MAKE distclean

test -f ../src
test -f ../sdir/src2
test -f ../nonmk-subdir/src3
test -f ../sdir/mk-subdir/src4

test -r src && exit 1
test -r sdir/src2 && exit 1
test -r nonmk-subdir/src3 && exit 1
test -r sdir/mk-subdir/src4 && exit 1

cd ..
./configure

$MAKE test

# Make sure nothing is deleted by 'make distclean'
$MAKE distclean

test -f src
test -f sdir/src2
test -f nonmk-subdir/src3
test -f sdir/mk-subdir/src4

:
