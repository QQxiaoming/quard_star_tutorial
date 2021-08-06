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

# Test of names in tar file.
# From Rainer Orth.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_SRCDIR([a.c])
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = wish
wish_SOURCES = a.c
END

: > a.c

$ACLOCAL
$AUTOCONF
$AUTOMAKE --include-deps --copy --add-missing
./configure
$MAKE dist

gzip -d $me-1.0.tar.gz
tar tf $me-1.0.tar > stdout || { cat stdout; exit 1; }
cat stdout
$FGREP ./$me-1.0/a.c stdout && exit 1

:
