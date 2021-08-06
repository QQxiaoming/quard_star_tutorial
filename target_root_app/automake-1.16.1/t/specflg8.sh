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

# Like the true/false example from the manual,
# with one extra indirection in the sources (PR/315), and
# use of _CPPFLAGS (PR/337).

required='cc native'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

# Using a separate variable to hold all the sources for a program is
# common when building many flavors of this program, each with
# different flags.

cat > Makefile.am << 'END'
TRUESOURCE = true.c
bin_PROGRAMS = false true
true_SOURCES = $(TRUESOURCE)
true_CPPFLAGS = -DEXIT_CODE=0
false_SOURCES = $(TRUESOURCE)
false_CPPFLAGS = -DEXIT_CODE=1
END

cat > true.c << 'END'
int main (void)
{
   return EXIT_CODE;
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE

./true
./false && exit 1

objext=$(sed -n -e 's/^OBJEXT = //p' < Makefile)
test -f ./true-true.$objext
test -f ./false-true.$objext

:
