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

# Test to make sure LIBOBJS file cannot be mentioned explicitly.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR
AC_LIBOBJ([fsusage])
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_LIBRARIES = libtu.a
libtu_a_SOURCES = fsusage.c
libtu_a_LIBADD = @LIBOBJS@
END

: > fsusage.c
: > ar-lib

$ACLOCAL
AUTOMAKE_fails

grep 'automatically discovered.*fsusage\.c.*explicitly mentioned' stderr

:
