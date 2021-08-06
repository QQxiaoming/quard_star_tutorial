#! /bin/sh
# Copyright (C) 2004-2018 Free Software Foundation, Inc.
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

# Check the tar-ustar option.

. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE([tar-ustar])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

: > Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure

grep 'am__tar' Makefile # For debugging.
if grep '^am__tar = false' Makefile; then
  skip_ "cannot find proper archiver program"
fi

$MAKE distcheck
test -f "$distdir.tar.gz"

:
