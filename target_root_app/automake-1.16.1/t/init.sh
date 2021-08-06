#! /bin/sh
# Copyright (C) 2006-2018 Free Software Foundation, Inc.
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

# Make sure we give a sensible error message when AC_INIT and
# AM_INIT_AUTOMAKE are both given less than two arguments.

. test-init.sh

for ac_init_args in '' '([x])'; do
  for am_init_args in '' '([1.10])'; do
    rm -rf aclocal.m4 autom4te*.cache
    unindent >configure.ac <<END
      AC_INIT$ac_init_args
      AM_INIT_AUTOMAKE$am_init_args
END
    cat configure.ac # For debugging.
    # The error message should mention AC_INIT, not AC_PACKAGE_VERSION.
    ($ACLOCAL && $AUTOCONF) 2>stderr && { cat stderr >&2; exit 1; }
    cat stderr >&2
    $FGREP AC_PACKAGE_VERSION stderr && exit 1
    grep 'configure\.ac:.* AC_INIT .*arguments' stderr
  done
done

:
