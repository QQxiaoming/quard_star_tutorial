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

# Test remake rules when an m4 file gets removed and the macros it
# defined get inlined into the caller.  Try with both an indirect
# call and a direct one.  This can be seen as testing the "deleted
# header file" issue w.r.t. aclocal.m4 dependencies.  See also
# related test 'aclocal-deleted-header.sh'.

. test-init.sh

cat >> configure.ac <<'END'
FOO_MACRO
AC_OUTPUT
END

cat > Makefile.am <<'END'
ACLOCAL_AMFLAGS = -I m4
.PHONY: test
test:
	test '$(the_answer)' -eq 42
END

macro_value='the_answer=42; AC_SUBST([the_answer])'

mkdir m4

cat > m4/foo.m4 <<'END'
AC_DEFUN([FOO_MACRO], [BAR_MACRO])
END

cat > m4/bar.m4 <<END
AC_DEFUN([BAR_MACRO], [$macro_value])
END

$ACLOCAL -I m4
$AUTOCONF
$AUTOMAKE

./configure
$MAKE test

$sleep

sed -e "s|BAR_MACRO|$macro_value|" m4/foo.m4 > t
mv -f t m4/foo.m4
rm -f m4/bar.m4

using_gmake || $MAKE Makefile
$MAKE test

$sleep

sed -e "s|FOO_MACRO|$macro_value|" configure.ac > t
mv -f t configure.ac
rm -f m4/foo.m4

using_gmake || $MAKE Makefile
$MAKE test

:
