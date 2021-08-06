#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Make sure we complain if @LTALLOCA@ is used without being set in configure.ac

. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AM_PROG_AR
END

cat > Makefile.am << 'END'
noinst_LTLIBRARIES = libtu.la
libtu_la_SOURCES =
libtu_la_LIBADD = @LTALLOCA@
END

: > alloca.c
: > ar-lib

$ACLOCAL
AUTOMAKE_fails
grep '^Makefile\.am:1:.*define.*LIBTOOL.* add .*LT_INIT' stderr
grep '^Makefile\.am:3:.*LTALLOCA' stderr

:
