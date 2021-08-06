#! /bin/sh
# Copyright (C) 2006-2018 Free Software Foundation, Inc.
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

# PR 492: Test asm subdir-objects.

required='gcc -c-o'
. test-init.sh

cat > configure.ac << END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE([subdir-objects])

AM_PROG_AS
AM_PROG_AR
AC_PROG_RANLIB

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_LIBRARIES = libfoo.a libbar.a
libfoo_a_SOURCES = src/a.s b.s
libbar_a_SOURCES = src/c.s d.s
libbar_a_CCASFLAGS =
END

mkdir src
: >src/a.s
: >b.s
: >src/c.s
: >d.s

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE
$MAKE distcheck

:
