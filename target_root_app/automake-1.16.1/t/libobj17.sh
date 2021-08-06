#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Make sure AC_LIBOBJ accept non-literal arguments.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR
foo=${FOO-oops}
AC_LIBSOURCES([quux.c, zardoz.c])
AC_LIBOBJ([$foo])
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_LIBRARIES = libtu.a
libtu_a_SOURCES =
libtu_a_LIBADD = $(LIBOBJS)
.PHONY: check-quux check-zardoz
check-quux:
	$(AR) t libtu.a | grep quux
	$(AR) t libtu.a | grep zardoz && exit 1; exit 0
check-zardoz:
	$(AR) t libtu.a | grep zardoz
	$(AR) t libtu.a | grep quux && exit 1; exit 0
END

cat > quux.c <<'END'
extern int dummy;
END

cat > zardoz.c <<'END'
extern int dummy;
END

# These might print warnings, but should not error out.
$ACLOCAL
$AUTOMAKE -a
$AUTOCONF

./configure FOO=quux
$MAKE
ls -l # For debugging.
test -f libtu.a
$MAKE check-quux

$MAKE distclean

./configure FOO=zardoz
$MAKE
ls -l # For debugging.
test -f libtu.a
$MAKE check-zardoz

:
