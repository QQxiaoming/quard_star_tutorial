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

# Make sure when using SUBDIRS that all BUILT_SOURCES are built.
# A bug occurred where subdirs do not have all-recursive or
# all-recursive-am which depended on BUILT_SOURCES.

required=cc
. test-init.sh

mkdir lib

cat >> configure.ac << 'END'
AC_CONFIG_FILES([lib/Makefile])
AC_PROG_RANLIB
AC_PROG_CC
AM_PROG_AR
AC_OUTPUT
END

cat > Makefile.am << 'END'
SUBDIRS = lib
END

cat > lib/Makefile.am << 'END'
pkgdata_DATA =
noinst_LIBRARIES = libfoo.a
libfoo_a_SOURCES = foo.c
BUILT_SOURCES = foo.h
foo.h:
	echo \#define FOO_DEFINE 1 >$@
CLEANFILES = $(BUILT_SOURCES)
END

cat > lib/foo.c << 'END'
#include <foo.h>
int foo (void) { return !FOO_DEFINE; }
END


$ACLOCAL
$AUTOCONF
$AUTOMAKE --copy --add-missing

./configure
$MAKE
$MAKE distcheck

:
