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

# Test of subdir make distclean rules.
# From Robert Collins.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = wish
wish_SOURCES = generic/a.c

test-all: all
	test -f generic/$(am__dirstamp)
test-mostlyclean: mostlyclean
	test ! -f generic/a.o
test-distclean: distclean
	test ! -f generic/$(am__dirstamp)
END

mkdir generic
cat > generic/a.c << 'END'
#include <stdio.h>
int main ()
{
   printf ("maude!\n");
   return 0;
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --include-deps --copy --add-missing

mkdir build
cd build

../configure
$MAKE test-all
$MAKE test-mostlyclean
$MAKE test-distclean

cd ..

# Now test without the subdir.
cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = wish
wish_SOURCES = a.c
END

mv generic/a.c a.c

$ACLOCAL
$AUTOCONF
$AUTOMAKE --include-deps --copy --add-missing

mkdir build2
cd build2

../configure
$MAKE
$MAKE mostlyclean

:
