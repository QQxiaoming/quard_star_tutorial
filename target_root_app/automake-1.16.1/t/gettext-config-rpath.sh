#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Check the config.rpath requirement.

required='gettext'
. test-init.sh

cat >> configure.ac << 'END'
AM_GNU_GETTEXT
# config.rpath is required by versions >= 0.14.3.
AM_GNU_GETTEXT_VERSION([0.14.3])
AC_OUTPUT
END

echo 'SUBDIRS = po intl' >Makefile.am
mkdir po intl

# If aclocal fails here, it may be that gettext is too old to provide
# AM_GNU_GETTEXT_VERSION.  Similarly, autopoint will fail if it's
# from an older version.  If gettext is too old to provide autopoint,
# this will fail as well, so we're safe here.
if ! $ACLOCAL && autopoint -n; then
  skip_ "too old gettext installation"
fi

AUTOMAKE_fails --add-missing
grep '^configure\.ac:.*required file.*config.rpath' stderr
: > config.rpath
$AUTOMAKE

:
