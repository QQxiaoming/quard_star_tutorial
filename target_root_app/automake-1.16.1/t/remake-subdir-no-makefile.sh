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

# Ensure that remake rules work for files in a subdirectory even when
# there is no Makefile for that subdirectory.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([sub/foo])
AC_OUTPUT
END

cat > Makefile.am <<'END'
all-local: sub/foo
END

mkdir sub
: > sub/foo.in

$ACLOCAL
$AUTOCONF
$AUTOMAKE

# First a VPATH build.
mkdir build
cd build
../configure
$sleep
test -f sub/foo
test ! -s sub/foo
echo '#GrepMe#' > ../sub/foo.in
$MAKE
grep '#GrepMe#' sub/foo

cd ..

# Now try an in-tree build.
: > sub/foo.in # Restore its original (empty) content.
./configure
$sleep
test -f sub/foo
test ! -s sub/foo
echo '#GrepMe#' > sub/foo.in
$MAKE
grep '#GrepMe#' sub/foo

# Finally try to check the distribution.
$sleep
echo '%GrepMe%' > sub/foo.in
cat >>Makefile.am <<END
check-local:
	grep '%GrepMe%' sub/foo
END
$MAKE
grep "grep '%GrepMe%' sub/foo" Makefile.in # Sanity check.
$MAKE distcheck

:
