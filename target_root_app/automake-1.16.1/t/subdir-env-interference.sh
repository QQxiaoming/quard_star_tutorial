#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# SUDBIRS with $fail set in the environment.

. test-init.sh

mkdir sub

cat >> configure.ac <<'END'
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

echo SUBDIRS = sub >Makefile.am
: > sub/Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure
env fail=1 $MAKE all clean

:
