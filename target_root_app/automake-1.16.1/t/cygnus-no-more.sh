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

# Check that support for Cygnus-style trees is deprecated.
# That feature will be removed in the next major Automake release.
# See automake bug#11034.

. test-init.sh

warn_rx='support for Cygnus.*trees.*removed'

$ACLOCAL

# Use of 'cygnus' option must raise an unconditional error, not a
# warning.
AUTOMAKE="$am_original_AUTOMAKE -Wnone -Wno-error"

# 'cygnus' option in Makefile.am
echo "AUTOMAKE_OPTIONS = cygnus" > Makefile.am
AUTOMAKE_fails
grep "^Makefile\.am:1:.*$warn_rx" stderr

rm -rf autom4te*.cache

# 'cygnus' option in configure.ac
: > Makefile.am
sed "s|^\\(AM_INIT_AUTOMAKE\\).*|\1([cygnus])|" configure.ac > t
diff configure.ac t && fatal_ "failed to edit configure.ac"
mv -f t configure.ac
AUTOMAKE_fails
grep "^configure\.ac:2:.*$warn_rx" stderr

:
