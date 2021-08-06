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

# Test to make sure Automake doesn't abort on user-defined extensions.
# Based on a report from Dmitry Mikhin <dmitrym@acres.com.au>.

# Also make sure that .SUFFIXES is automatically adjusted with
# extensions from implicit rules.

. test-init.sh

cat > Makefile.am << 'END'
.k.o:
	echo $< > $@

bin_PROGRAMS = foo
foo_SOURCES = foo.k
END

$ACLOCAL
$AUTOMAKE
grep '^\.SUFFIXES:' Makefile.in | sed -e 's/$/ /' > suffixes
cat suffixes
$FGREP ' .k ' suffixes

:
