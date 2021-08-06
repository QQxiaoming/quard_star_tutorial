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

# Test to make sure we can compile when the compiler doesn't
# understand '-c -o'.

required=gcc # For cc-no-c-o.
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
$CC --version || exit 1
$CC -v || exit 1
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = wish
wish_SOURCES = a.c
# Make sure we need something strange.
wish_CFLAGS = -g
END

cat > a.c << 'END'
#include <stdio.h>

int main ()
{
   printf ("hi\n");
}
END

# Make sure the compiler doesn't understand '-c -o'.
CC=$am_testaux_builddir/cc-no-c-o; export CC

$ACLOCAL
$AUTOCONF -Wall -Werror
$AUTOMAKE --copy --add-missing

for vpath in : false; do
  if $vpath; then
    srcdir=..
    mkdir build
    cd build
  else
    srcdir=.
  fi
  $srcdir/configure >stdout || { cat stdout; exit 1; }
  cat stdout
  $EGREP 'understands? -c and -o together.* no$' stdout
  # No repeated checks please.
  test $(grep -c ".*-c['\" ].*-o['\" ]" stdout) -eq 1
  $MAKE
  cd $srcdir
done

$MAKE distcheck

:
