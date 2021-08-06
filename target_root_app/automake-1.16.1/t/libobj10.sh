#! /bin/sh
# Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

# Do not complain about the nonexistence of a source for LIBOBJS if
# it's in BUILT_SOURCES.  Reported by Erez Zadok.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_LIBOBJ([foo])
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_LIBRARIES = libfoo.a
libfoo_a_SOURCES =
libfoo_a_LIBADD = $(LIBOBJS)
BUILT_SOURCES = foo.c
CLEANFILES = foo.c
foo.c:
	echo 'extern int dummy;' > $@
.PHONY: debugging
debugging:
	$(AR) t libfoo.a
END

cp "$am_scriptdir/ar-lib" . || fatal_ "fetching auxiliary script 'ar-lib'"

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

$MAKE
$MAKE debugging
$MAKE distcheck

:
