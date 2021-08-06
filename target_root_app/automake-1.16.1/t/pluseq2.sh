#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Test '+=' with conditionals.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([CHECK], [true])
END

cat > Makefile.am << 'END'

if CHECK
data_DATA = zar
else
data_DATA =
endif

if CHECK
data_DATA += doz
else
data_DATA += dog
endif

END

$ACLOCAL
$AUTOMAKE
grep 'CHECK_TRUE.*zar doz' Makefile.in
grep 'CHECK_FALSE.*dog' Makefile.in

:
