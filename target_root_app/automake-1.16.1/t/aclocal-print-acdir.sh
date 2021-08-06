#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Test on aclocal's '--print-ac-dir' option.

am_create_testdir=empty
. test-init.sh

$ACLOCAL --print-ac-dir
test "$($ACLOCAL --print-ac-dir)" = "$am_system_acdir"

$ACLOCAL --system-acdir /bar --print-ac-dir
test "$($ACLOCAL --system-acdir /bar --print-ac-dir)" = /bar

$ACLOCAL --automake-acdir /bar --print-ac-dir
test "$($ACLOCAL --automake-acdir /bar --print-ac-dir)" = "$am_system_acdir"

:
