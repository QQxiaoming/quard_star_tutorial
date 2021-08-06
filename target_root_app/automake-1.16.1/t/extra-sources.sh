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

# Test to make sure EXTRA_..._SOURCES actually works.
# Bug report from Henrik Frystyk Nielsen.

. test-init.sh

echo AC_PROG_CC >> configure.ac

cat > Makefile.am <<'END'
bin_PROGRAMS = www
www_SOURCES = www.c
EXTRA_www_SOURCES = xtra.c
www_LDADD = @extra_stuff@
END

$ACLOCAL
$AUTOMAKE

grep '@am__include@ .*/xtra\.Po' Makefile.in
grep '^am__depfiles_remade =.*/xtra.Po' Makefile.in

:
