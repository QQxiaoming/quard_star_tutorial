#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# AM_PROG_GCJ should cause OBJEXT and EXEEXT to be set.

required='gcj'
. test-init.sh

cat >> configure.ac << 'END'
AM_PROG_GCJ
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_PROGRAMS = test
test_SOURCES = Test.java
test_LDFLAGS = --main=Test
END

cat >Test.java << 'END'
public class Test {
    public static void main(String[] argv) {
        System.out.println("Hello, automake!");
    }
}
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF
./configure

# These fail without e.g., AC_PROG_CC.
$MAKE
grep "OBJEXT = " Makefile
grep "EXEEXT = " Makefile

:
