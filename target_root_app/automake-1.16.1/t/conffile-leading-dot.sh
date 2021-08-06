#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Automake must complain if AC_CONFIG_FILES is passed something starting
# with a dot (like "./Makefile"), since the remake rules might be subtly
# broken in that case.

required=GNUmake
. test-init.sh

cat > configure.ac << END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([./Makefile])
AC_CONFIG_FILES([./foo:a.in:b.in:c.in])
AC_OUTPUT
END

touch a.in b.in c.in Makefile.am

$ACLOCAL

AUTOMAKE_fails -Wnone -Wunsupported
grep "^configure\.ac:3:.*'\\./Makefile'" stderr
grep "^configure\.ac:3:.* omit leading '\\./'" stderr
grep "^configure\.ac:3:.*remake rules might be subtly broken" stderr
grep "^configure\.ac:4:.*'\\./foo'" stderr
grep "^configure\.ac:4:.* omit leading '\\./'" stderr
grep "^configure\.ac:4:.*remake rules might be subtly broken" stderr

# Check that our warning was actually justified.
sed 's/^AM_INIT_AUTOMAKE/&([-Wall -Wno-unsupported])/' <configure.ac >t
mv -f t configure.ac
rm -rf autom4te*.cache
$ACLOCAL
$AUTOCONF
$AUTOMAKE -Wall -Wno-unsupported
./configure
$MAKE
$sleep
touch Makefile.am
# Check that remake rules do truly break -- otherwise automake is
# giving a bogus warning.
run_make -e FAIL -E
grep "config\\.status:.*invalid argument.*Makefile" stderr

:
