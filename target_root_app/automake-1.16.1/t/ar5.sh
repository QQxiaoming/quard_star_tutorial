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

# Test the optional argument of AM_PROG_AR.

. test-init.sh

cat >> configure.ac << 'END'
AM_PROG_AR([
  echo spy > bad-archiver-interface-detected
  AC_MSG_CHECKING([for something else])
  AC_MSG_RESULT([found it])
  echo Grep This
])
END

$ACLOCAL
$AUTOCONF

./configure AR=/bin/false >stdout || { cat stdout; exit 1; }
cat stdout
grep '^checking.* archiver .*interface.*\.\.\. unknown$' stdout
grep '^checking for something else\.\.\. found it$' stdout
grep '^Grep This$' stdout
test -f bad-archiver-interface-detected

:
