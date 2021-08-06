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

# Check that automake and autoconf complain about an old-style AC_INIT
# call used with a new-style AM_AUTOMAKE_INIT call.

. test-init.sh

: > Makefile.am

# We'll need a proper aclocal.m4, but aclocal is expected to fail
# later, so get and store it now.
$ACLOCAL
mv aclocal.m4 aclocal-m4.sav

errmsg='AC_INIT should be called with package and version argument'

for ac_init in AC_INIT 'AC_INIT([Makefile.am])'; do
  for am_init_automake in AM_INIT_AUTOMAKE 'AM_INIT_AUTOMAKE([1.11])'; do
    rm -rf autom4te*.cache aclocal.m4
    unindent > configure.ac <<END
        $ac_init
        $am_init_automake
        AC_CONFIG_FILES([Makefile])
END
    cat configure.ac # For debugging.
    $ACLOCAL 2>stderr && { cat stderr >&2; exit 1; }
    cat stderr >&2
    grep "^configure\\.ac:.* $errmsg" stderr
    cp aclocal-m4.sav aclocal.m4
    $AUTOCONF 2>stderr && { cat stderr >&2; exit 1; }
    cat stderr >&2
    grep "^configure\\.ac:.* $errmsg" stderr
    AUTOMAKE_fails
    grep "^configure\\.ac:.* $errmsg" stderr
  done
done

:
