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

# Test that some Java-related variables and rules are not repeatedly
# defined.

. test-init.sh

cat > Makefile.am << 'END'
javadir = $(datadir)/java
java_JAVA = a.java
dist_java_JAVA = b.java
nodist_java_JAVA = c.java
nobase_java_JAVA = d.java
nobase_dist_java_JAVA = e.java
nobase_nodist_java_JAVA = f.java
END

$ACLOCAL
$AUTOMAKE

$EGREP -i '\.stamp|\.class|java|classpath' Makefile.in # For debugging.

for var in JAVAC JAVAROOT CLASSPATH_ENV am__java_sources; do
  grep "^$var =" Makefile.in
  test $(grep -c "^[$sp$tab]*$var[$sp$tab]*=" Makefile.in) -eq 1
done

grep '^classjava\.stamp:' Makefile.in
test $(grep -c "class.*java.*\.stamp.*:" Makefile.in) -eq 1

:
