#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Make sure multiple ":"s in AC_CONFIG_FILES do not generate broken
# rebuild rules.
# Test from Maciej W. Rozycki.

. test-init.sh

cat >> configure.ac <<END
AC_CONFIG_FILES([zardoz:one:two:three])
AC_OUTPUT
END

: > Makefile.am
: > one
: > two
: > three

$ACLOCAL
$AUTOMAKE

# The rule should regenerate the file "zardoz", not a meaningless
# file "'zardoz:one:two".
$FGREP 'zardoz:one:two' Makefile.in && exit 1
exit 0
