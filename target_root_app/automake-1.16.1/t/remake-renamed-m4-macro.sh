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

# Test remake rules when the name of an m4 macro change.  Try both with
# and without indirection.

. test-init.sh

cat >> configure.ac <<'END'
MY_MACRO
AC_OUTPUT
END

cat > Makefile.am <<'END'
ACLOCAL_AMFLAGS = -I m4
.PHONY: test
test:
	test '$(the_answer)' -eq 42
END

mkdir m4

cat > m4/macros.m4 <<'END'
AC_DEFUN([MY_MACRO], [FOO_1])
END

cat > m4/foo.m4 <<'END'
AC_DEFUN([FOO_1], [the_answer=42
                   AC_SUBST([the_answer])])
END

$ACLOCAL -I m4
$AUTOCONF
$AUTOMAKE

./configure
$MAKE test

$sleep

for x in macros foo; do
  sed -e 's/FOO_1/FOO_2/' m4/$x.m4 > t
  mv -f t m4/$x.m4
done

using_gmake || $MAKE Makefile
$MAKE test

$sleep

for f in m4/macros.m4 configure.ac; do
  sed -e 's/MY_MACRO/YOUR_MACRO/' $f > t
  mv -f t $f
done

using_gmake || $MAKE Makefile
$MAKE test

:
