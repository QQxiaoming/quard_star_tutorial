#! /bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

# Test to make sure LIBOBJS works in subdirs.
# Bug from Josh MacDonald.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([subdir/Makefile])
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_CHECK_TOOLS([AR], [ar])
AC_LIBOBJ([fsusage])
AC_OUTPUT
END

cat > Makefile.am << 'END'
SUBDIRS = subdir
check-local: distdir
	ls -l $(srcdir) $(srcdir)/subdir
	ls -l $(distdir) $(distdir)/subdir
	ls -l $(builddir) $(builddir)/subdir
	test -f $(srcdir)/subdir/fsusage.c
	test -f $(distdir)/subdir/fsusage.c
	$(AR) tv $(builddir)/subdir/libtu.a
END

mkdir subdir

cat > subdir/Makefile.am << 'END'
noinst_LIBRARIES = libtu.a
libtu_a_SOURCES =
libtu_a_LIBADD = @LIBOBJS@
END

cat > subdir/fsusage.c << 'END'
extern int dummy;
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

# Older grepping check, kept "just to be sure".
$FGREP 'fsusage.c' subdir/Makefile.in

./configure

$MAKE check
$MAKE distcheck

:
