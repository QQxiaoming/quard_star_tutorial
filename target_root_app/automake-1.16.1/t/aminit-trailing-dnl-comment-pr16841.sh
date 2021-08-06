#! /bin/sh
# Copyright (C) 2014-2018 Free Software Foundation, Inc.
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

# Check that a trailing 'dnl' m4 comment automake after the
# AM_INIT_AUTOMAKE invocation doesn't produce a syntactically
# invalid configure script.  This used to be the case until
# automake 1.13, but we broke that in automake 1.14. See
# automake bug#16841.

am_create_testdir=empty
. test-init.sh

cat > configure.ac <<END
AC_INIT([test-pr16841], [1.0])
AM_INIT_AUTOMAKE([1.14 -Werror]) dnl Some comment
echo "OK OK OK"
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

: > Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure >stdout || { cat stdout; exit 1; }
cat stdout
grep '^OK OK OK$' stdout

:
