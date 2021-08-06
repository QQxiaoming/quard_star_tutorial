#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Make sure AC_LIBSOURCE and AC_LIBSOURCES use arguments literally.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
foo=dummy bar=dummy baz=dummy
AC_LIBSOURCE([$foo.c])
AC_LIBSOURCES([$bar.c, $baz.c])
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_LIBRARIES = libtu.a
libtu_a_SOURCES =
libtu_a_LIBADD = $(LIBOBJS)
END

: > dummy.c

$ACLOCAL
AUTOMAKE_fails
grep 'configure\.ac:.*required file.*\$foo\.c.*not found' stderr
grep 'configure\.ac:.*required file.*\$bar\.c.*not found' stderr
grep 'configure\.ac:.*required file.*\$baz\.c.*not found' stderr

:
