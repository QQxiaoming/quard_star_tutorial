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

# Test error reporting for AC_CONFIG_LIBOBJ_DIR.
# See also sister tests 'libobj20a.sh' and 'libobj20c.sh'.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_LIBOBJ_DIR([libobj-dir])
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_LIBSOURCE([foobar.c])
# NOTE: this call to AC_OUTPUT is really needed; see Automake bug #7635
#       <https://debbugs.gnu.org/cgi/bugreport.cgi?bug=7635>
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
noinst_LIBRARIES = libtu.a
libtu_a_SOURCES =
libtu_a_LIBADD = $(LIBOBJS)
END

: > ar-lib

$ACLOCAL

AUTOMAKE_fails
grep 'configure\.ac:.*required directory.*libobj-dir' stderr

mkdir libobj-dir
: > foobar.c # Oops, it should be in libobj-dir!
AUTOMAKE_fails
grep 'configure\.ac:.*required file.*libobj-dir/foobar.c.*' stderr

rm -f foobar.c

: > libobj-dir/foobar.c
$AUTOMAKE # Now we should succeed.

:
