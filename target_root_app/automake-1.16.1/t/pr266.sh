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

# Test for PR 266.
# Dependency tracking -vs- nonstandard Makefile names

required=cc
. test-init.sh

cat > configure.ac << END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_PROG_CC
AC_CONFIG_FILES([Maudefile])
AC_OUTPUT
END

cat > Maudefile.am << 'END'
include_HEADERS = 3dfx.h linutil.h
noinst_HEADERS  = fx64.h fxdll.h fximg.h fxglob.h \
                  fxos.h fxver.h glob.h
noinst_PROGRAMS = fxmisc
fxmisc_SOURCES  = fx64.c fximg.c fxos.c linutil.c
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF
./configure --enable-dependency-tracking

test -f .deps/fx64.Po

:
