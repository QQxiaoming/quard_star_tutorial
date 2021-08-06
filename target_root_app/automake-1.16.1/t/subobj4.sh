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

# Make sure '../foo/foo.cpp' generates a rule.
# Report from Dave Brolley.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_CXX
AC_CONFIG_FILES([d1/Makefile d2/Makefile])
AC_OUTPUT
END

mkdir d1 d2

cat > Makefile.am << 'END'
SUBDIRS = d1 d2
END

cat > d1/Makefile.am << 'END'
bin_PROGRAMS = z
z_SOURCES = ../d2/z.c
END

cat > d2/Makefile.am << 'END'
END

: > d2/z.c

$ACLOCAL
$AUTOMAKE -Wno-unsupported

grep '\$(CC) .*\.\./d2/z\.c' d1/Makefile.in

:
