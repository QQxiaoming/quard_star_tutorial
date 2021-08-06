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

# Automake gettext support: regression check for PR/381:
# 'SUBDIRS = po intl' must not be required if 'po/' doesn't exist.

required='gettext'
. test-init.sh

cat >> configure.ac << 'END'
AM_GNU_GETTEXT
AC_OUTPUT
END

$ACLOCAL

: > config.guess
: > config.rpath
: > config.sub

test ! -d po # Sanity check.
mkdir sub
echo 'SUBDIRS = sub' > Makefile.am
$AUTOMAKE

# Still, SUBDIRS must be defined.

: > Makefile.am
AUTOMAKE_fails
grep '^configure\.ac:.*AM_GNU_GETTEXT used but SUBDIRS not defined' stderr

:
