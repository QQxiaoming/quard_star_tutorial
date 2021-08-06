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

# Test for multiple replacement functions.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
# 'am__dummy_function' is there to ensure that at least one function is
# replaced, to avoid creating an empty archive which can cause problems
# with e.g. Solaris ar.
AC_REPLACE_FUNCS([basename dirname am__dummy_function])
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_LIBRARIES = libtu.a
libtu_a_SOURCES =
libtu_a_LIBADD = @LIBOBJS@
check-local: test1 test2 test3
.PHONY: test1 test2 test3
test1: all
	$(AR) tv libtu.a
test2:
	@echo DIST_COMMON = $(DIST_COMMON)
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]basename\.c '
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]dirname\.c '
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]am__dummy_function\.c '
test3: distdir
	test -f $(distdir)/basename.c
	test -f $(distdir)/dirname.c
	test -f $(distdir)/am__dummy_function.c
END

cat > basename.c <<'END'
extern int dummy1;
END
cat > dirname.c <<'END'
extern int dummy2;
END
cat > am__dummy_function.c <<'END'
extern int dummy3;
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure

$MAKE
$MAKE check
$MAKE distcheck

:
