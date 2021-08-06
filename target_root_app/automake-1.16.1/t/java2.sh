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

# Make sure that Java rules doesn't attempt to run javac when there is
# no classes to compile.
# Report from Braden McDaniel.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL(WITH_JDK, false)
AC_OUTPUT
END

cat > Makefile.am << 'END'
JAVAC = false
JAVAROOT = $(top_builddir)/java

JAVA_FILES = MyClass1.java

if WITH_JDK
noinst_JAVA = $(JAVA_FILES)
endif

EXTRA_DIST = $(JAVA_FILES)
END

: >MyClass1.java

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure
$MAKE
