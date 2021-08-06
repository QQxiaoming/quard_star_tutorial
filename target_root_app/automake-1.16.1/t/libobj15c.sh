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

# Nonexistent sources for AC_LIBSOURCES should cause Automake to fail.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_RANLIB
AC_LIBSOURCES([foobar.c, bazquux.c])
# NOTE: this call to AC_OUTPUT is really needed; see Automake bug #7635
#       <https://debbugs.gnu.org/cgi/bugreport.cgi?bug=7635>
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_LIBRARIES = libtu.a
libtu_a_SOURCES =
libtu_a_LIBADD = $(LIBOBJS)
END

# Don't trip on errors due to missing 'AM_PROG_AR'.
AUTOMAKE="$AUTOMAKE -Wno-extra-portability"

$ACLOCAL

AUTOMAKE_fails
grep 'configure\.ac:.*required file.*foobar\.c.*' stderr
grep 'configure\.ac:.*required file.*bazquux\.c.*' stderr

: > foobar.c
AUTOMAKE_fails
grep 'configure\.ac:.*required file.*bazquux\.c.*' stderr
grep 'foobar\.c' stderr && exit 1

:
