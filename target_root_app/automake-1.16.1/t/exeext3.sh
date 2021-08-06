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

# Make sure we can override a program rule.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
END

cat > Makefile.am << 'END'
bin_PROGRAMS = maude 3dldf

maude$(EXEEXT):
	yeah

3dldf$(EXEEXT):
	yippie
END

$ACLOCAL
$AUTOMAKE -Wno-override

$FGREP 'maude$(EXEEXT):' Makefile.in
test 1 -eq $(grep -c 'maude.*:' Makefile.in)
$FGREP '3dldf$(EXEEXT):' Makefile.in
test 1 -eq $(grep -c '3dldf.*:' Makefile.in)

:
