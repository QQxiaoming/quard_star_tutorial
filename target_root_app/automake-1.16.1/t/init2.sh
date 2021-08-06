#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Make sure we give a sensible error message when AM_INIT_AUTOMAKE
# contains junk.

. test-init.sh

cat >configure.ac <<END
AC_INIT([init2], [1.0])
AM_INIT_AUTOMAKE([nosuchoption])
END

$ACLOCAL
AUTOMAKE_fails
grep 'nosuchoption.*recognized' stderr
exit 0
