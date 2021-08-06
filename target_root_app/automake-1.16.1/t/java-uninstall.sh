#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Check uninstallation of Java class files.

required=javac
. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
javadir = $(prefix)/java
java_JAVA = Foo.java
nobase_java_JAVA = Foo2.java
nobase_dist_java_JAVA = Bar.java
nodist_java_JAVA = Baz.java

# Java files are not distributed by default, so we distribute
# one "by hand" ...
EXTRA_DIST = Foo.java
# ... and make the other one generated.
Foo2.java:
	rm -f $@ $@-t
	echo 'class bClass {}' > $@-t
	chmod a-w $@-t && mv -f $@-t $@

# Explicitly declared as 'nodist_', so generate it.
Baz.java:
	rm -f $@ $@-t
	echo 'class Baz {}' > $@-t
	echo 'class Baz2 {}' >> $@-t
	chmod a-w $@-t && mv -f $@-t $@

DISTCLEANFILES = Baz.java Foo2.java
END

echo 'class aClass {}' > Foo.java
echo 'class Zardoz {}' > Bar.java

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure --prefix="$(pwd)"/_inst
javadir=_inst/java

check_uninstallation()
{
  test ! -e $javadir/aClass.class
  test ! -e $javadir/bClass.class
  test ! -e $javadir/Zardoz.class
  test ! -e $javadir/Baz.class
  test ! -e $javadir/Baz2.class
  test   -f $javadir/Foo.class
  test   -f $javadir/Bar.class
  test   -f $javadir/xClass.class
}

$MAKE
ls -l
$MAKE install
: > $javadir/Foo.class
: > $javadir/Bar.class
: > $javadir/xClass.class
ls -l $javadir
$MAKE uninstall
ls -l $javadir
check_uninstallation

# FIXME: "make uninstall" should continue to work also after "make clean",
#        but currently this doesn't happen.  See automake bug#8540.
$MAKE install
ls -l $javadir
$MAKE clean
ls -l
$MAKE uninstall
ls -l $javadir
#check_uninstallation

$MAKE distcheck

:
