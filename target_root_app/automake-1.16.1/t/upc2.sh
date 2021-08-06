#! /bin/sh
# Copyright (C) 2006-2018 Free Software Foundation, Inc.
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

# Test that Automake suggests using AM_PROG_UPC if Unified Parallel C
# sources are used.

. test-init.sh

echo AC_PROG_CC >>configure.ac

cat >Makefile.am <<'END'
bin_PROGRAMS = hello
hello_SOURCES = hello.upc
END

$ACLOCAL
AUTOMAKE_fails
grep AM_PROG_UPC stderr
