#! /bin/sh
# Copyright (C) 2008-2018 Free Software Foundation, Inc.
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

# Check for AM_MAINTAINER_MODE defaults.

. test-init.sh

cat >> configure.ac << 'END'
AM_MAINTAINER_MODE
AC_OUTPUT
END

: >Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
grep '^MAINT.*#' Makefile

./configure --disable-maintainer-mode
grep '^MAINT.*#' Makefile

./configure --enable-maintainer-mode
grep '^MAINT.*#' Makefile && exit 1

sed 's/\(AM_MAINTAINER_MODE\).*/\1([disable])/' configure.ac > configure.tmp
mv -f configure.tmp configure.ac
mv configure configure1
$AUTOCONF --force
diff configure configure1

sed 's/\(AM_MAINTAINER_MODE\).*/\1([enable])/' configure.ac > configure.tmp
mv -f configure.tmp configure.ac
$AUTOCONF --force

./configure
grep '^MAINT.*#' Makefile && exit 1

./configure --enable-maintainer-mode
grep '^MAINT.*#' Makefile && exit 1

./configure --disable-maintainer-mode
grep '^MAINT.*#' Makefile

sed 's/\(AM_MAINTAINER_MODE\).*/\1([foo])/' configure.ac > configure.tmp
mv -f configure.tmp configure.ac
$AUTOCONF --force -Werror && exit 1

:
