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

# Make sure that check_JAVA causes *.class files to be built only with
# "make check", and not also with "make all".
# See automake bug#8234.

required=javac
. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
check_JAVA = One.java Two.java
END

cat > One.java <<'END'
class One { }
END

cat > Two.java <<'END'
class Two { // Deliberately missing closing bracket.
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

inst=$(pwd)/_inst

./configure --prefix="$inst"

$MAKE
ls | $EGREP '\.(class|stamp)$' && exit 1

# Make Two.java compilable.
echo '}' >> Two.java

# "make check" should compile files in $(check_JAVA) ...
$MAKE check
ls -l # For debugging.
test -f One.class
test -f Two.class
# ... but should *not* install them.
$FGREP checkdir Makefile && exit 1
$MAKE install
test -d _inst && exit 1

:
