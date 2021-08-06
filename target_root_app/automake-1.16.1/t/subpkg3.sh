#! /bin/sh
# Copyright (C) 2006-2018 Free Software Foundation, Inc.
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

# Make sure different subpackages may share files and directories.

. test-init.sh

cat >Makefile.am <<'END'
SUBDIRS = subpkg
EXTRA_DIST = subpkg/foobar
END

cat >>configure.ac <<'END'
AC_CONFIG_SUBDIRS([subpkg])
AC_OUTPUT
END

mkdir subpkg

echo foobar >subpkg/foobar

echo SUBDIRS = >subpkg/Makefile.am

cat >subpkg/configure.ac <<'END'
AC_INIT([subpkg], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

cd subpkg
$ACLOCAL
$AUTOMAKE
$AUTOCONF
cd ..

./configure
$MAKE distcheck
