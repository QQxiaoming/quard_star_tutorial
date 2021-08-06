#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Make sure empty calls to AC_CONFIG_FILES or AC_CONFIG_HEADERS are diagnosed.

. test-init.sh

# We avoid using configure.ac stub initialized by our testsuite setup, as
# we need to keep track of line numbers (to grep for error messages).
cat > configure.ac << END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES ([oops])
AC_CONFIG_HEADERS
AC_OUTPUT
END

$ACLOCAL
AUTOMAKE_fails
grep 'configure\.ac:3:.* arguments .*AC_CONFIG_FILES' stderr
grep 'configure\.ac:4:.* arguments .*AC_CONFIG_HEADERS' stderr

:
