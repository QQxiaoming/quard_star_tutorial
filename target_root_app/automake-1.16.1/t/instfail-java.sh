#! /bin/sh
# Copyright (C) 2008-2018 Free Software Foundation, Inc.
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

# The install rule should honor failures of the install program.
# Some of these are already caught by 'instmany.sh'.

# This is the java sister test of 'instfail.sh'.

required=javac
. test-init.sh

cat >>configure.ac <<'END'
AC_OUTPUT
END

cat >Makefile.am <<'END'
javadir = $(datadir)/java
java_JAVA = java1.java java2.java java3.java
END

for n in 1 2 3; do
  echo "class java$n { }" >java$n.java
done

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

instdir=$(pwd)/inst || fatal_ "getting current working directory"
./configure --prefix="$instdir"
$MAKE

$MAKE install
$MAKE uninstall

for file in java1.class
do
  chmod a-r $file
  test ! -r $file || skip_ "cannot drop file read permissions"
  $MAKE install-data && exit 1
  chmod u+r $file
done

:
