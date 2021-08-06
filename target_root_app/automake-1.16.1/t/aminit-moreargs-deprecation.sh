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

# Check that automake warns against old-style usages of AM_INIT_AUTOMAKE
# (i.e., calls with two or three arguments).

. test-init.sh

warn_rx='AM_INIT_AUTOMAKE.* two-.* three-arguments form.*deprecated'

cat > configure.ac <<'END'
AC_INIT([Makefile.am])
AM_INIT_AUTOMAKE([twoargs], [1.0])
AC_CONFIG_FILES([Makefile])
END

$ACLOCAL

do_check()
{
  rm -rf autom4te*.cache
  for cmd in "$AUTOCONF" "$AUTOMAKE"; do
    $cmd -Werror -Wnone -Wobsolete 2>stderr && { cat stderr; exit 1; }
    cat stderr >&2
    grep "^configure\.ac:2:.*$warn_rx" stderr
    $cmd -Werror -Wall -Wno-obsolete || exit 1
  done
}

: > Makefile.am
do_check

sed "/^AM_INIT_AUTOMAKE/s|)$|, [NODEFINE])|" configure.ac > t
diff configure.ac t && fatal_ "failed to edit configure.ac"
mv -f t configure.ac
do_check

:
