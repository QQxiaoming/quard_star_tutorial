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

# Check that the JAVA primary can be used freely in the same Makefile.am
# with proper combinations of the 'dist_', 'nodist_' and 'nobase_'
# modifiers.  Also check that '.java' files are not distributed by
# default.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
# This test does not require compilation of *.java files.
JAVAC = false
javadir = $(prefix)

java_JAVA = Class.java
dist_java_JAVA = ClassDist.java
nodist_java_JAVA = ClassNodist.java

nobase_java_JAVA = ClassNobase.java
nobase_dist_java_JAVA = ClassNobaseDist.java
nobase_nodist_java_JAVA = ClassNobaseNodist.java

Class.java ClassNodist.java ClassNobase.java ClassNobaseNodist.java:
	@echo '$@ should not be generated!' >&2; exit 1
END

: > ClassDist.java
: > ClassNobaseDist.java

$ACLOCAL
$AUTOCONF
# Automake used to display non-fatal warnings with this test, but
# they were unexpected, so we want to consider them as failures in
# this test.
$AUTOMAKE 2>stderr || { cat stderr >&2; exit 1; }
cat stderr >&2
test ! -s stderr

./configure

$MAKE distdir
ls -l $distdir # For debugging.
test ! -e $distdir/Class.java
test -f $distdir/ClassDist.java
test ! -e $distdir/ClassNobase.java
test -f $distdir/ClassNobaseDist.java
test ! -e $distdir/ClassNodist.java
test ! -e $distdir/ClassNobaseNodist.java

:
