#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Make sure --help and --version work, even when no configure.ac nor
# configure.in is in the current directory.

. test-init.sh

# Ensure we run in an empty directory.
mkdir emptydir
cd emptydir

# Honour user overrides for $ACLOCAL and $AUTOMAKE, but without
# adding extra options.
ACLOCAL=$am_original_ACLOCAL
AUTOMAKE=$am_original_AUTOMAKE

$ACLOCAL --version
$ACLOCAL --help
$AUTOMAKE --version
$AUTOMAKE --help

# Sanity checks: aclocal and automake cannot work without configure.ac
# nor configure.in.
$ACLOCAL 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
$FGREP configure.ac stderr
AUTOMAKE_fails
$FGREP configure.ac stderr

:
