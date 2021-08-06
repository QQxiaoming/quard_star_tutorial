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

# Test cleaning of Java class files and timestamps.

required=javac
. test-init.sh

cat >> configure.ac <<'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
javadir = $(datadir)/java

java_JAVA = Class.java
dist_java_JAVA = ClassDist.java
nodist_java_JAVA = ClassNodist.java

nobase_java_JAVA = ClassNobase.java
nobase_dist_java_JAVA = ClassNobaseDist.java
nobase_nodist_java_JAVA = ClassNobaseNodist.java
END

for base in '' Nobase; do
  for dist in '' Dist Nodist; do
    echo "class Class$base$dist {}" > Class$base$dist.java
  done
done

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE
ls -l
test -f classjava.stamp
for base in '' Nobase; do
  for dist in '' Dist Nodist; do
    test -f Class$base$dist.class
  done
done
$MAKE clean
find . -name '*.class' -o -name '*.stamp' | grep . && exit 1
# We should not remove unrelated stamp files.
echo timestamp > classjava2.stamp
echo timestamp > classdist_java.stamp
$MAKE clean
test -f classjava2.stamp
test -f classdist_java.stamp

:
