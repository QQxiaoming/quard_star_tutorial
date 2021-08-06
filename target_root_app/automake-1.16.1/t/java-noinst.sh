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

# Make sure that noinst_JAVA causes generated *.class files not to
# be installed.

required=javac
. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am <<'END'
noinst_JAVA = 1.java
dist_noinst_JAVA = 2.java
nodist_noinst_JAVA = 3.java
nobase_noinst_JAVA = 4.java
nobase_dist_noinst_JAVA = 5.java
nobase_nodist_noinst_JAVA = 6.java
END

for i in 1 2 3 4 5 6; do
  echo "class Class$i {}" > $i.java
done

$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure --prefix="$(pwd)/_inst"

$MAKE
ls -l
for i in 1 2 3 4 5 6; do
  test -f Class$i.class
done

$MAKE install
test -d _inst && { ls -l _inst; exit 1; }

:
