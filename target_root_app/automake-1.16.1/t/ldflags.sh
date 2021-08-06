#! /bin/sh
# Copyright (C) 2000-2018 Free Software Foundation, Inc.
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

# Check for LDFLAGS in conditional.
# PR 77.

required=libtool
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_LIBTOOL
AC_SUBST([LTLIBOBJS], [q.o])
AM_CONDITIONAL([USE_SWIG], [:])
AC_OUTPUT
END

cat > Makefile.am << 'END'
if USE_SWIG
lib_LTLIBRARIES = libtu.la
libtu_la_SOURCES = foo.c
libtu_la_LDFLAGS = -module
endif
END

: > ltconfig
: > ltmain.sh
: > ar-lib
: > config.guess
: > config.sub
: > q.c

$ACLOCAL
$AUTOMAKE

:
