#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Check that the AM_PROG_MKDIR_P macro is deprecated.  It will be
# be removed in the next major Automake release.

. test-init.sh

echo AM_PROG_MKDIR_P >> configure.ac
: > Makefile.am

grep_err ()
{
  loc='^configure.ac:4:'
  grep "$loc.*AM_PROG_MKDIR_P.*deprecated" stderr
  grep "$loc.* use .*AC_PROG_MKDIR_P" stderr
  grep "$loc.* use '\$(MKDIR_P)' instead of '\$(mkdir_p)'.*Makefile" stderr
}

$ACLOCAL

$AUTOCONF -Werror -Wobsolete 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
grep_err

$AUTOCONF -Werror -Wno-obsolete

#AUTOMAKE_fails
#grep_err
AUTOMAKE_fails --verbose -Wnone -Wobsolete
grep_err

$AUTOMAKE -Wno-obsolete

:
