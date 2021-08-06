#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Check that the first among --help and --version to be specified on
# the command line takes precedence over the following one.

. test-init.sh

# Ensure we run in an empty directory.
mkdir emptydir
cd emptydir

# Honour user overrides for $ACLOCAL and $AUTOMAKE, but without
# adding extra options.
ACLOCAL=$am_original_ACLOCAL
AUTOMAKE=$am_original_AUTOMAKE

escape_dots () { sed 's/\./\\./g'; } # Avoid issues with "\" in backquotes.
apiversion_rx=$(echo "$APIVERSION" | escape_dots)

$ACLOCAL --version --help >stdout || { cat stdout; exit 1; }
cat stdout
grep "^aclocal.*$apiversion_rx" stdout
grep "^Usage" stdout && exit 1

$ACLOCAL --help --version >stdout || { cat stdout; exit 1; }
cat stdout
grep "^Usage" stdout
grep "^aclocal.*$apiversion_rx" stdout && exit 1

$AUTOMAKE --version --help >stdout || { cat stdout; exit 1; }
cat stdout
grep "^automake.*$apiversion_rx" stdout
grep "^Usage" stdout && exit 1

$AUTOMAKE --help --version >stdout || { cat stdout; exit 1; }
cat stdout
grep "^Usage" stdout
grep "^automake.*$apiversion_rx" stdout && exit 1

:
