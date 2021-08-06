#!/bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Ensure we warn about substitutions in bin_PROGRAMS if EXTRA_PROGRAMS
# are missing; but only if the former is not AC_SUBSTed itself
# (lib_LIBRARIES is in the same boat here).

. test-init.sh

cat >>configure.ac <<'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_SUBST([lib_LIBRARIES])
AC_SUBST([bins])
AC_OUTPUT
END

cat >Makefile.am <<'END'
bin_PROGRAMS = @bins@
END

: > ar-lib

$ACLOCAL
AUTOMAKE_fails
grep 'bin_PROGRAMS.*contains configure substitution' stderr
grep 'lib_LIBRARIES.*contains configure substitution' stderr && exit 1

:
