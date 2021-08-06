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

# The 'cscope' target should not fail if there are no sources in a subdir.
# In practice this means 'cscope' should not be invoked if cscope.files
# is empty.

required=cscope
. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END
mkdir sub
echo 'SUBDIRS = sub' >Makefile.am
: >sub/Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE cscope

:
