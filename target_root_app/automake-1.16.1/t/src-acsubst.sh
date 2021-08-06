#! /bin/sh
# Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

# Test to make sure config substitution in _SOURCES fails.

. test-init.sh

echo AC_PROG_CC >> configure.ac

cat > Makefile.am << 'END'
bin_PROGRAMS = x y
x_SOURCES = x.c @FOO@
bar = @FOO@
foo = $(bar)
EXTRA_y_SOURCES = $(foo) y.c
END

$ACLOCAL
AUTOMAKE_fails

cat > exp-err << 'END'
Makefile.am:2: error: 'x_SOURCES' includes configure substitution '@FOO@';
Makefile.am:2: configure substitutions are not allowed in _SOURCES variables
Makefile.am:3: error: 'bar' includes configure substitution '@FOO@'
Makefile.am:3: and is referred to from 'EXTRA_y_SOURCES';
Makefile.am:3: configure substitutions are not allowed in _SOURCES variables
END

diff exp-err stderr

:
