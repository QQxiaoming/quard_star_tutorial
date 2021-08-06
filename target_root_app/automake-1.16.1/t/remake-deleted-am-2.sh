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
# of the "deleted header problem".  This test checks deeper inclusion
# stacks, and use VPATH builds.

. test-init.sh

echo AC_OUTPUT >> configure.ac

$ACLOCAL
$AUTOCONF

cat > Makefile.am <<'END'
include $(srcdir)/foo.am
END

cat > foo.am <<'END'
# this is foo
include sub/bar.am
include baz.am
END

mkdir sub
echo 'include $(top_srcdir)/sub/zardoz.am' > sub/bar.am
echo '# this is zardoz' > sub/zardoz.am

echo 'include fnord.am' > baz.am
echo '# you are not seeing this' > fnord.am

$AUTOMAKE
# Sanity checks.
$FGREP 'this is zardoz' Makefile.in
$FGREP 'you are not seeing this' Makefile.in

mkdir build
cd build

srcdir=..

$srcdir/configure
$MAKE

$sleep
echo '# this is baz' > $srcdir/baz.am
rm -f $srcdir/fnord.am
$MAKE
# Sanity checks.
$FGREP 'you are not seeing this' $srcdir/Makefile.in Makefile && exit 1
$FGREP 'this is baz' $srcdir/Makefile.in
$FGREP 'this is baz' Makefile

$sleep
rm -rf $srcdir/sub $srcdir/foo.am $srcdir/baz.am
echo '# no more inclusions' > $srcdir/Makefile.am
$MAKE
# Sanity checks.
$EGREP 'this is (foo|bar|baz)' Makefile $srcdir/Makefile.in && exit 1
$FGREP 'no more inclusions' Makefile

:
