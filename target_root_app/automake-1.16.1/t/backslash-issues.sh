#! /bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

# Test for "\" problems.
# TODO: might be nice to convert this to TAP...

. test-init.sh

echo AC_PROG_CC >> configure.ac
$ACLOCAL

# Bug report from Joerg-Martin Schwarz.
cat > Makefile.am << 'END'
bin_PROGRAMS = \
   frob
END
$AUTOMAKE
grep '^_SOURCE' Makefile.in && exit 1

# We must skip the backslash, not complain about './\' not existing.
# Reported by Rick Scott <rwscott@omnisig.com>
cat > Makefile.am << 'END'
SUBDIRS = \
   .
END
$AUTOMAKE

# Make sure we diagnose trailing backslash at the end of a file.
# Report from Akim Demaile <akim@epita.fr>.
echo 'foo = \' > Makefile.am
AUTOMAKE_fails
grep 'trailing backslash' stderr

:
