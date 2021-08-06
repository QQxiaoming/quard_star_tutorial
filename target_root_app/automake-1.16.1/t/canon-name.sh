#! /bin/sh
# Copyright (C) 2007-2018 Free Software Foundation, Inc.
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

# PR 511: Make sure we warn about e.g. AC_CONFIG_FILES([./gmakefile]),
# as not all make implementations treat 'file' and './file' equally.

. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([./gmakefile ./sub/gmakefile])
AC_OUTPUT
END

mkdir sub
echo 'SUBDIRS = sub' >gmakefile.am
: >sub/gmakefile.am

$ACLOCAL
AUTOMAKE_fails
grep 'leading.*\./.*\./gmakefile' stderr
grep 'leading.*\./.*\./sub/gmakefile' stderr

:
