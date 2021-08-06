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

# Test on automake options '--print-data-dir' and '--print-script-dir'.

am_create_testdir=empty
. test-init.sh

libdir=$($AUTOMAKE --print-libdir) || exit 1
case $libdir in /*);; *) exit 1;; esac
test -d "$libdir"
test "$libdir" = "$am_pkgvdatadir"
test "$libdir" = "$am_scriptdir"

: > Makefile.am

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
END

touch ChangeLog README NEWS AUTHORS COPYING

$ACLOCAL
$AUTOMAKE --gnu --add-missing
ls -l

diff missing "$libdir"/missing
diff INSTALL "$libdir"/INSTALL

:
