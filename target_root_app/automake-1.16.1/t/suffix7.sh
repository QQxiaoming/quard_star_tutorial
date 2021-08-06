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

# Test to make sure Automake supports implicit rules "confusing"
# extensions.  Inspired by a mail from Alex Hornby.

. test-init.sh

cat > Makefile.am << 'END'
SUFFIXES = .idl S.cpp C.h
SUFFIXES += C.cpp S.h
.idlC.cpp:
	cp $< $@
END

$ACLOCAL
$AUTOMAKE

# Make sure Automake has NOT recognized .cpp and .idlC as two new
# extensions.
grep 'SUFFIXES.* \.cpp' Makefile.in && exit 1
grep 'SUFFIXES.* \.idlC' Makefile.in && exit 1

:
