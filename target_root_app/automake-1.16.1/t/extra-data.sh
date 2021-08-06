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

# Checks on the obsolete EXTRA_DATA variable.

. test-init.sh

echo 'AC_SUBST([CODICIL])' >> configure.ac

$ACLOCAL

# EXTRA_DATA is not required ....
echo sysconf_DATA = @CODICIL@ > Makefile.am
$AUTOMAKE

# ... but it can nonetheless be specified.
echo EXTRA_DATA = codicil.txt >> Makefile.am
$AUTOMAKE

:
