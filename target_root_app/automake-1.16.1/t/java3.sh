#! /bin/sh
# Copyright (C) 2004-2018 Free Software Foundation, Inc.
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

# Make sure that Java rules don't attempt to compile or install
# *.java files when there are none.
# Report from Johannes Nicolai (PR/441).

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([WHO_CARES], [false])
AC_OUTPUT
END

cat > Makefile.am << 'END'
if WHO_CARES
JAVA_FILES = MyClass1.java
endif

javadir = $(prefix)/java
java_JAVA = $(JAVA_FILES)
END

cat > MyClass1.java << 'END'
class MyClass1 { // Deliberately missing right curly bracket.
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure --prefix="$(pwd)/_inst"
$MAKE
$MAKE install
ls -l .
find . -name '*.class' | grep . && exit 1
# If we have nothing to install, we shouldn't create any installation
# directory.  Related to automake bug#11030.
test ! -e _inst
$MAKE uninstall
$MAKE distcheck

:
