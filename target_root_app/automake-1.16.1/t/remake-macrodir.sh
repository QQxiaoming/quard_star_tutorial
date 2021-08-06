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

# Check that the aclocal honouring of AC_CONFIG_MACRO_DIR interacts
# nicely with automatic rebuild rules.

. test-init.sh

cat >> configure.ac <<'END'
AC_CONFIG_MACRO_DIR([macro-dir])
my__FOO || AS_EXIT([1])
AC_OUTPUT
END

: > Makefile.am

mkdir macro-dir
cat > macro-dir/foo.m4 <<'END'
AC_DEFUN([my__FOO], [: > bar])
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

$FGREP my__FOO configure && exit 1

./configure
test -f bar
rm -f bar

$sleep

cat > macro-dir/foo.m4 <<'END'
AC_DEFUN([my__FOO], [: > baz])
END

$MAKE Makefile
test -f baz
test ! -r bar
rm -f baz

$sleep

rm -f macro-dir/foo.m4
cat > macro-dir/quux.m4 <<'END'
AC_DEFUN([my__FOO], [: > quux])
END

$MAKE Makefile
test -f quux
test ! -r baz
rm -f quux

$sleep

sed 's/^AC_CONFIG_MACRO_DIR/&([newdir])/' configure.ac > t
mv -f t configure.ac

mkdir newdir
cat > newdir/mu.m4 <<'END'
AC_DEFUN([my__FOO], [[: my__FOO do nothing]])
END

$MAKE Makefile
$FGREP ': my__FOO do nothing' configure

$MAKE distcheck

:
