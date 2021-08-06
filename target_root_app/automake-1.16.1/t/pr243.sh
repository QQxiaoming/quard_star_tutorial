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

# Test for PR 243.
# AM_OUTPUT_DEPENDENCY_COMMANDS doesn't handle
# 'Makefile:Makefile.in:tail.mk' in AC_OUTPUT.
#
# == Report ==
# If configure.ac has something like:
#       AC_OUTPUT(Makefile:Makefile.in:tail.mk)
# then config.status cannot parse the Makefile to build the
# dependency files in the .deps directory.  This is because
# the AM_OUTPUT_DEPENDENCY_COMMANDS macro cannot cope with
# the colon in the CONFIG_FILES variable.

required=cc
. test-init.sh

cat > configure.ac << END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_PROG_CC
AC_CONFIG_FILES([Makefile:Makefile.in:tail.mk])
AC_OUTPUT
END

: > tail.mk

cat > Makefile.am << 'END'
include_HEADERS  = 3dfx.h linutil.h
noinst_HEADERS   = fx64.h fxdll.h fximg.h fxglob.h \
                   fxos.h fxver.h glob.h
noinst_PROGRAMS = fxmisc
fxmisc_SOURCES  = fx64.c fximg.c fxos.c linutil.c
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF
./configure

test -f .deps/fx64.Po

:
