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

# Make sure AC_LIBOBJ and friends work.
# Please keep this in sync with sister test 'libobj16b.sh'.

required=cc
. test-init.sh

mv configure.ac configure.proto
cat >> configure.proto << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
%LIBOBJ-STUFF% # Will be activated later.
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_LIBRARIES = libtu.a
libtu_a_SOURCES =
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
liver-not-dist: distdir
	test ! -r $(distdir)/liver.c
maude-not-dist: distdir
	test ! -r $(distdir)/maude.c
maude-lib:
	$(AR) t libtu.a | grep maude
maude-not-lib:
	$(AR) t libtu.a | grep maude && exit 1; exit 0
liver-lib:
	$(AR) t libtu.a | grep liver
liver-not-lib:
	$(AR) t libtu.a | grep liver && exit 1; exit 0
END

cat > maude.c << 'END'
extern int dummy_maude;
END

cat > liver.c << 'END'
extern int dummy_liver;
END

sed '/%LIBOBJ-STUFF%/{
s/.*//
i\
AC_LIBOBJ([maude])
}' configure.proto > configure.ac
cat configure.ac # For debugging.

cat > extra-checks.am << 'END'
extra_checks = maude-src maude-dist maude-lib liver-not-dist
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure

$MAKE
$MAKE check
$MAKE distcheck
$MAKE distclean

# Avoid timestamp-related differences.
rm -rf autom4te*.cache

cat > extra-checks.am << 'END'
extra_checks = maude-src maude-dist liver-src liver-dist
if MAUDE_COND
extra_checks += maude-lib liver-not-lib
else
extra_checks += maude-not-lib liver-lib
endif
END

sed '/%LIBOBJ-STUFF%/{
s/.*//
i\
AM_CONDITIONAL([MAUDE_COND], [test x"$MAUDE" = x"yes"])\
if test x"$MAUDE" = x"yes"; then\
  AC_LIBOBJ([maude])\
else\
  AC_LIBOBJ([liver])\
fi\
AC_LIBSOURCES([maude.c, liver.c])
}' configure.proto > configure.ac
cat configure.ac # For debugging.

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure MAUDE=yes
$MAKE
$MAKE check
$MAKE distcheck
$MAKE distclean

./configure MAUDE=no
$MAKE
$MAKE check
$MAKE distcheck
$MAKE distclean

:
