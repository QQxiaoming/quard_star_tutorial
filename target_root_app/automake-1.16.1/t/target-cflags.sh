#! /bin/sh
# Copyright (C) 2000-2018 Free Software Foundation, Inc.
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

# Test to make sure target specific CFLAGS work
# Assar Westerlund <assar@sics.se>

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = no-dependencies

bin_PROGRAMS = foo bar
foo_CFLAGS   = -DFOO
END

cat > foo.c << 'END'
#include <stdio.h>
#ifdef FOO
int main(void)
{
  return 0;
}
#endif
END

cat > bar.c << 'END'
#ifndef FOO
int main(void)
{
  return 0;
}
#endif
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
mkdir build

cd build
../configure
$MAKE
if ! cross_compiling; then
  ./foo
  ./bar
fi
cd ..

./configure
$MAKE
if ! cross_compiling; then
  ./foo
  ./bar
fi

:
