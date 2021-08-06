#! /bin/sh
# Copyright (C) 1997-2018 Free Software Foundation, Inc.
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

# Check to make sure incorrect LDADD usage is diagnosed.

required=libtool
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_LIBTOOL
AC_SUBST([LTLIBOBJS], [q.lo])
AC_OUTPUT
END

cat > Makefile.am << 'END'
lib_LTLIBRARIES = libtu.la
libtu_la_SOURCES = foo.c
libtu_la_LDADD = @LTLIBOBJS@
END

: > ltconfig
: > ltmain.sh
: > config.guess
: > config.sub
: > q.c

$ACLOCAL || exit 1
AUTOMAKE_fails -Wno-extra-portability
grep "libtu_la_LDADD" stderr
grep " use 'libtu_la_LIBADD'" stderr

:
