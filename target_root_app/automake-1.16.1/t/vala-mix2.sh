#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Vala sources, C and C++ sources and C and C++ headers in the same
# program.  Functional test.  See automake bug#10894.

required='valac cc c++ pkg-config GNUmake'
. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AC_PROG_CXX
AM_PROG_VALAC([0.7.3])
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0 >= 2.4])
AC_OUTPUT
END

cat > Makefile.am <<'END'
bin_PROGRAMS = zardoz
AM_CFLAGS = $(GOBJECT_CFLAGS)
zardoz_LDADD = $(GOBJECT_LIBS)
zardoz_SOURCES = zardoz.vala foo.h bar.c baz.c zen.hh master.cxx
END

cat > zardoz.vala <<'END'
int main ()
{
  stdout.printf ("foo is alive\n");
  return 0;
}
END

cat > foo.h <<'END'
int foo;
int bar (void);
int baz (void);
END

cat > bar.c <<'END'
#include "foo.h"
int bar (void) { return foo + baz (); }
END

cat > baz.c <<'END'
#include "foo.h"
extern int foo = 0;
int baz (void) { return 0; }
END

cat > zen.hh <<'END'
#include <iostream>
END

cat > master.cxx <<'END'
#include "zen.hh"
void chatty (void) { std::cout << "Hello, stranger!\n"; }
END

$ACLOCAL
$AUTOMAKE -a
$AUTOCONF

# Do not reject slower dependency extractors.
./configure --enable-dependency-tracking

$MAKE all
ls -l # For debugging.

have_generated_files ()
{
  test -f zardoz_vala.stamp
  test -f zardoz.c
}

# Our vala-related rules must create stamp files and intermediate
# C files.
have_generated_files

# Remake rules are not uselessly triggered.
$MAKE -q
$MAKE -n | $FGREP vala.stamp && exit 1

# But are triggered when they should.
for file in zardoz.vala foo.h bar.c baz.c zen.hh master.cxx; do
  $sleep
  echo '& choke me !' >> $file
  $MAKE && exit 1
  $sleep
  sed '$d' $file > t
  mv -f t $file
  $MAKE
done

# Check the distribution.
$MAKE distcheck

# Stamp files and intermediate C files should *not* be removed
# by "make clean".
$MAKE clean
ls -l # For debugging.
have_generated_files

# But stamp files should be removed by "maintainer-clean" (the
# behaviour w.r.t. intermediate C files is still unclear, and
# better left undefined for the moment).
$MAKE maintainer-clean
ls *vala*.stamp | grep . && exit 1

:
