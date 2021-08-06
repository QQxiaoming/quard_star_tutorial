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

# Test support for AC_CONFIG_LIBOBJ_DIR.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_LIBOBJ_DIR([libobj-dir])
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_LIBOBJ([foobar])
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
noinst_LIBRARIES = libtu.a
libtu_a_SOURCES =
libtu_a_LIBADD = $(LIBOBJS)
## Hack with this variable ans with extra make recursion in the check-local
## rule are required for when we move this Makefile in a subdir, later.
my_distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)
check-local:
	(cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) distdir)
	ls -l $(top_srcdir)/* $(top_builddir)/* $(my_distdir)/*
	test -f libtu.a
	test ! -r $(top_srcdir)/foobar.c
	test -f $(top_srcdir)/libobj-dir/foobar.c
	test ! -r $(my_distdir)/foobar.c
	test -f $(my_distdir)/libobj-dir/foobar.c
	$(AR) t libtu.a
	$(AR) t libtu.a | grep foobar
END

mkdir libobj-dir
cat > libobj-dir/foobar.c << 'END'
extern int dummy;
END

cp "$am_scriptdir/ar-lib" . || fatal_ "fetching auxiliary script 'ar-lib'"

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

$MAKE
$MAKE check
$MAKE distcheck

# Same check, but with the Makefile.am using $(LIBOBJS) not being
# the top-level one.

$MAKE distclean
rm -rf autom4te*.cache aclocal.m4 configure

mkdir sub
mv -f Makefile.am sub
echo SUBDIRS = sub > Makefile.am

sed '/^AC_OUTPUT/i\
AC_CONFIG_FILES([sub/Makefile])
' configure.ac > t
mv -f t configure.ac
cat configure.ac # For debugging.

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

$MAKE
$MAKE check
$MAKE distcheck

:
