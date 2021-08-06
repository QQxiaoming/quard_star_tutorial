#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Test if a file can be mentioned in LIBOBJS and explicitly.
# (See libobj13.sh for the LTLIBRARIES check.)

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_LIBOBJ([foo])
AC_LIBOBJ([bar])
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_LIBRARIES = libfoo.a libbar.a
noinst_PROGRAMS = p1 p2

libfoo_a_SOURCES =
libfoo_a_LIBADD = @LIBOBJS@

libbar_a_SOURCES = foo.c

p1_SOURCES =
p1_LDADD = @LIBOBJS@

p2_SOURCES = bar.c
END

: > ar-lib
: > foo.c
: > bar.c

$ACLOCAL

# This however should be diagnosed, since foo.c and bar.c are in @LIBOBJS@.
cat >> Makefile.am << 'END'
libfoo_a_SOURCES += foo.c
p1_SOURCES += bar.c
END

AUTOMAKE_fails
grep 'foo\.c.*explicitly mentioned' stderr
grep 'bar\.c.*explicitly mentioned' stderr

# Global 'LDADD' can also come into play.
cat > Makefile.am << 'END'
noinst_PROGRAMS = a b
LDADD = @LIBOBJS@
END

$AUTOMAKE
grep 'a_DEPENDENCIES.*LIBOBJS' Makefile.in

cat >> Makefile.am << 'END'
a_SOURCES = foo.c
END
AUTOMAKE_fails
grep 'foo\.c.*explicitly mentioned' stderr

:
