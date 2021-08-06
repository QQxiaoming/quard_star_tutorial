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

# Check diagnostics about no-installman being disallowed in 'gnu'.

. test-init.sh

cat > configure.ac << 'END'
AC_INIT([gnuwarn2], [1.0])
AM_INIT_AUTOMAKE([no-installman])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

# --gnu files
: > NEWS
: > README
: > AUTHORS
: > ChangeLog
: > COPYING
: > INSTALL

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = gnu
END

$ACLOCAL
AUTOMAKE_fails
grep 'configure.ac:2:.*no-installman' stderr
