#!/bin/sh
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

# Test that libtool support works correctly when "newer" libtool
# interface (with LT_INIT etc.) is used (this interface has been
# present since libtool 1.9b, circa 2004).

required='cc libtoolize'
. test-init.sh

cat >>configure.ac <<'END'
AC_PROG_CC
AM_PROG_AR
dnl Older libtool versions don't define LT_PREREQ :-(
m4_ifdef([LT_PREREQ],
    [LT_PREREQ([2.0])],
    [m4_fatal([Libtool version too old], [63])])
LT_INIT([dlopen])
AC_OUTPUT
END

cat >Makefile.am <<'END'
lib_LTLIBRARIES = libfoo.la
END

cat > libfoo.c <<'END'
int foo (void)
{
  return 1;
}
END

libtoolize
# Skip if older libtool (pre-2.0) is used.
{ $ACLOCAL && $AUTOCONF; } || {
  if test $? -eq 63; then
    skip_ "libtool is too old (probably < 2.0)"
  else
    exit 1
  fi
}
$EGREP 'LT_(INIT|PREREQ)' configure && exit 1 # Sanity check.
$AUTOMAKE -a

./configure --prefix="$(pwd)/inst" >stdout || { cat stdout; exit 1; }
cat stdout
grep '^checking.*dlfcn\.h.* no$' stdout || grep '^checking.*dlopen' stdout

$MAKE
$MAKE install
$MAKE distcheck

:
