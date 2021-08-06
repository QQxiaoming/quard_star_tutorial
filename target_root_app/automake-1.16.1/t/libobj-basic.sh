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

# Make sure AC_LIBSOURCE and AC_LIBSOURCES work.

required=cc
. test-init.sh

mv configure.ac configure.proto
cat >> configure.proto << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_LIBRARIES = libtu.a
libtu_a_SOURCES = foo.c
libtu_a_LIBADD = $(LIBOBJS)

include extra-checks.am
.PHONY: $(extra_checks) pre-test

pre-test: distdir
	ls -l $(srcdir) $(builddir) $(distdir)
	$(AR) tv libtu.a
$(extra_checks): pre-test
check-local: $(extra_checks)

maude-src:
	grep dummy_maude $(srcdir)/maude.c
maude-dist:
	grep dummy_maude $(distdir)/maude.c
liver-src:
	grep dummy_liver $(srcdir)/liver.c
liver-dist:
	grep dummy_liver $(distdir)/liver.c
liver-not-dist:
	test -d $(distdir)
	test ! -r $(distdir)/liver.c
maude-not-dist:
	test -d $(distdir)
	test ! -r $(distdir)/maude.c
END

cat > foo.c << 'END'
extern int dummy_foo;
END

cat > maude.c << 'END'
extern int dummy_maude;
END

cat > liver.c << 'END'
extern int dummy_liver;
END

# AC_LIBSOURCE should work also if called after AC_OUTPUT.
cat configure.proto - > configure.ac <<END
AC_LIBSOURCE([maude.c])
END

cat > extra-checks.am << 'END'
extra_checks = maude-src maude-dist liver-not-dist
END

cp "$am_scriptdir/ar-lib" . || fatal_ "fetching auxiliary script 'ar-lib'"

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure

$MAKE check
$MAKE distcheck

$MAKE distclean
# Avoid timestamp-related differences.
rm -rf autom4te*.cache

cat > extra-checks.am << 'END'
extra_checks = maude-src maude-dist liver-src liver-dist
END

# AC_LIBSOURCES should work also if called after AC_OUTPUT.
cat configure.proto - > configure.ac <<END
AC_LIBSOURCES([maude.c, liver.c])
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure

$MAKE check
$MAKE distcheck

:
