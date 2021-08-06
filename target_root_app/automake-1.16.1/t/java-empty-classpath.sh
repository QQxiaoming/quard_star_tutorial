#! /bin/sh
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

# Java compilation works also when CLASSPATH is unset or empty at
# compilation time.  See automake bug#9306.

required=javac
. test-init.sh

cat >> configure.ac <<'END'
AC_CONFIG_SRCDIR([org/gnu/bug/Library.java])
AC_CONFIG_FILES([
    org/Makefile
    org/gnu/Makefile
    org/gnu/bug/Makefile
])
AC_OUTPUT
END

mkdir org org/gnu org/gnu/bug
cat > Makefile.am <<END
CLEANFILES = *.class
SUBDIRS = org
END
echo SUBDIRS = gnu > org/Makefile.am
echo SUBDIRS = bug > org/gnu/Makefile.am
cat > org/gnu/bug/Makefile.am <<'END'
JAVAROOT = ../../..
dist_noinst_JAVA = Library.java Application.java
END

cat > org/gnu/bug/Library.java <<'END'
package org.gnu.bug;
public class Library
{
    public Library ()
    {
      // Nothing to do.
    }
    public static void doSomethingUseful (String arg)
    {
      System.out.println (arg);
    }
}
END

cat > org/gnu/bug/Application.java <<'END'
import org.gnu.bug.*;
public class Application
{
    public static void main (String args[])
    {
        Library lib = new Library ();
        lib.doSomethingUseful ("PLUGH");
    }
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure

unset CLASSPATH
$MAKE
$MAKE clean

CLASSPATH=''; export CLASSPATH
$MAKE
$MAKE clean

unset CLASSPATH
$MAKE distcheck

:
