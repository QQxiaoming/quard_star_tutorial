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

# Check use of EXTRA with the JAVA primary.  Also test interaction
# of JAVA with conditionals (it's natural to test it here, since
# EXTRA_JAVA exists mostly for ensuring interoperation with Automake
# conditionals).

required=javac
. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([COND], [test x"$cond" = x"yes"])
AC_OUTPUT
END

cat > Makefile.am << 'END'
javadir = $(pkgdatadir)/java

EXTRA_JAVA = Class1.java Class2.java Class3.java

java_JAVA = Class1.java

if COND
java_JAVA += Class2.java
else !COND
java_JAVA += Class3.java
endif !COND

Class3.java: Makefile
	echo 'class Class3 {}' > $@
CLEANFILES = Class3.java
END

echo "class Class1 {}" > Class1.java
echo "class Class2 {}" > Class2.java

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure cond=yes
$MAKE
ls -l
test -f Class1.class
test -f Class2.class
test ! -e Class3.class
test ! -e Class3.java

$MAKE distclean

./configure cond=no
$MAKE
ls -l
test -f Class1.class
test ! -e Class2.class
test -f Class3.class
test -f Class3.java

:
