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

# Test to make sure EXTRA_ targets are generated.  This test used to
# make sure the targets were *not* generated.  That is wrong; the
# targets should always be generated.  However, they should not be
# built by default.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
END

cat > Makefile.am << 'END'
bin_PROGRAMS = @foo@
EXTRA_PROGRAMS = zardoz
END

$ACLOCAL
$AUTOMAKE

grep "^zardoz" Makefile.in

# Can't have EXTRA clean rules.
grep 'clean.*EXTRA' Makefile.in && exit 1

:
