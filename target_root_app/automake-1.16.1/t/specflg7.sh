#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# The true/false example from the manual, plus a check for _SHORTNAME.

required='cc native'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = false true
true_SOURCES = false.c
true_CFLAGS = -DAM_TRUE
true_SHORTNAME = t
# No false_SOURCES definition.  Use the default source.
false_CFLAGS = -DAM_FALSE
false_SHORTNAME = f
END

cat > false.c << 'END'
#include <stdio.h>
int
main (int argc, char *argv[])
{
#ifdef AM_TRUE
   puts ("true");
#else
   puts ("false");
#endif
   return 0;
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE
./true | grep true
./false | grep false

objext=$(sed -n -e 's/^OBJEXT = //p' < Makefile)
test -f ./t-false.$objext
test -f ./f-false.$objext

:
