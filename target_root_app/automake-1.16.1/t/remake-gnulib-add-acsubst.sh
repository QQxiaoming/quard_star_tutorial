#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Test remake rules when a new AC_SUBST'd variable is added, and C header
# files are involved.
# This test overlaps with others, and is not strictly necessary per se,
# but it exercises a real use case (from gnulib, see:
#  <https://lists.gnu.org/archive/html/bug-gnulib/2011-04/msg00005.html>
# for more info).

required=cc
. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
MY_MACROS
AC_OUTPUT
END

cat > Makefile.am <<'END'
ACLOCAL_AMFLAGS = -I m4
noinst_PROGRAMS = foo
foo_SOURCES = foo.c
BUILT_SOURCES = foo.h
edit_h = sed -e 's|[@]foovar@|@foovar@|g'
foo.h: foo.in.h
	$(edit_h) < $(srcdir)/foo.in.h > $@-t
	cat $@-t;: For debugging.
	mv -f $@-t $@
EXTRA_DIST = foo.in.h
MOSTLYCLEANFILES = foo.h foo.h-t
END

mkdir m4

cat > m4/foo.m4 <<'END'
AC_DEFUN([MY_MACROS], [
     FOO_MACRO
dnl: ZAP_MACRO
])
END

cat > m4/bar.m4 <<'END'
AC_DEFUN([FOO_MACRO], [
     foovar=42; AC_SUBST([foovar])
dnl: barvar=47; AC_SUBST([barvar])
])
END

cat > foo.in.h <<'END'
#define foo @foovar@
END

cat > foo.c <<'END'
#include "foo.h"
int main (void) { return 0; }
typedef int checkfoo[1 - 2 * (foo != 42)];
END

$ACLOCAL -I m4
$AUTOCONF
$AUTOMAKE

./configure
$MAKE

: AC_SUBST @barvar@ and add it to foo.h.

$sleep

sed -e 's/^dnl:/   /' m4/bar.m4 > t
mv -f t m4/bar.m4
cat m4/bar.m4

cat >> foo.in.h <<'END'
#define bar @barvar@
END

cat >> foo.c <<'END'
typedef int checkbar[1 - 2 * (bar != 47)];
END

cat >> Makefile.am <<'END'
edit_h += -e 's|[@]barvar@|@barvar@|g'
END

using_gmake || $MAKE Makefile
$MAKE

: AC_SUBST @zapvar@ and add it to foo.h.
# Do it in a slightly different way from how it was done for @barvar@.

$sleep

cat >> Makefile.am <<'END'
edit_h += -e 's|[@]zapvar@|$(zapvar)|g'
END

cat >> foo.c <<'END'
typedef int checkzap[1 - 2 * (zap != 163)];
END

sed -e 's/^dnl://' m4/foo.m4 > t
mv -f t m4/foo.m4
cat m4/foo.m4

cat >> foo.in.h <<'END'
#define zap @zapvar@
END

cat >> m4/bar.m4 <<'END'
AC_DEFUN([ZAP_MACRO], [zapvar=163; AC_SUBST([zapvar])])
END

using_gmake || $MAKE Makefile
$MAKE

$MAKE distcheck

:
