#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Check that the obsolescent idiom of setting $(ACLOCAL_AMFLAGS) in
# Makefile.am still works.  Remove this test once support for this
# obsolescent idiom is removed.

. test-init.sh

cat >> configure.ac <<'END'
MACRO_FOO || AS_EXIT([1])
AC_OUTPUT
END

mkdir m4_1 m4_2
cat > m4_1/foo.m4 <<'END'
AC_DEFUN([MACRO_FOO], [: > foo])
END
cat > m4_2/bar.m4 <<'END'
AC_DEFUN([MACRO_BAR], [: > bar])
END

cat > Makefile.am <<'END'
ACLOCAL_AMFLAGS = -I m4_2 --verbose
check-local:
	test ! -r foo
	test -f bar
DISTCLEANFILES = bar
END

$ACLOCAL -I m4_1 >output 2>&1 || { cat output; exit 1; }
cat output
grep 'found macro' output && exit 1 # Sanity check.
$AUTOCONF
$AUTOMAKE

./configure
test -f foo
rm -f foo

# ACLOCAL_AMFLAGS is used in rebuild rules, so trigger them.
$sleep

sed 's/MACRO_FOO/MACRO_BAR/' configure.ac > t
mv -f t configure.ac

run_make -M Makefile
grep "^aclocal.*:.*found macro.*MACRO_BAR.*m4_2/bar\.m4" output
grep "macro.*MACRO_FOO" output && exit 1
test ! -r foo
test -f bar

$MAKE distcheck

:
