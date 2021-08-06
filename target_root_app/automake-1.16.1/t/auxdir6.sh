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

# Test to make sure AC_CONFIG_AUX_DIR works correctly.
# This test calls AC_CONFIG_AUX_DIR with an explicit literal argument,
# thus explicitly making the directory named by that argument the
# config auxdir.
# Keep this in sync with sister tests 'auxdir7.sh' and 'auxdir8.sh'.

. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AC_CONFIG_AUX_DIR([auxdir])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile subdir/Makefile])
END

mkdir subdir auxdir

cat > Makefile.am << 'END'
pkgdata_DATA =
END

cp Makefile.am subdir/Makefile.am

: > auxdir/mkinstalldirs
: > auxdir/install-sh
: > auxdir/missing

$ACLOCAL
$AUTOMAKE

$FGREP '$(top_srcdir)/auxdir/mkinstalldirs' Makefile.in
$FGREP '$(top_srcdir)/auxdir/mkinstalldirs' subdir/Makefile.in

:
