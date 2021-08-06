#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Make sure Automake does not emit a link rule for unknown languages.

. test-init.sh

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = no-exeext

bin_PROGRAMS = meal
meal_SOURCES = beans.veg beef.meat

meal: beans.veg beef.meat
	cat beans.veg beef.meat > meal
END

$ACLOCAL
$AUTOMAKE -Wno-override

grep '^meal.*:' Makefile.in | grep -v beef.meat && exit 1

exit 0
