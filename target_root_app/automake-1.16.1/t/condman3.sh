#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Make sure conditionals work with man pages.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([COND], [test x"$FOO" = x"true"])
AC_OUTPUT
END

cat > Makefile.am << 'END'
if COND
man_MANS = foo.1
man4_MANS = 6.man
else
man_MANS = bar.2 baz.1
man5_MANS = zap.5
endif

.PHONY: test1 test2
test1:
	find $(mandir) ;: For debugging.
	test -f $(mandir)/man1/foo.1
	test -f $(mandir)/man4/6.4
	test ! -f $(mandir)/man2/bar.2
	test ! -f $(mandir)/man1/baz.1
	test ! -f $(mandir)/man5/zap.5
test2:
	find $(mandir) ;: For debugging.
	test -f $(mandir)/man2/bar.2
	test -f $(mandir)/man1/baz.1
	test -f $(mandir)/man5/zap.5
	test ! -f $(mandir)/man1/foo.1
	test ! -f $(mandir)/man4/6.4
	test ! -f $(mandir)/man4/6.man
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

$EGREP 'MANS|\.([123456789]|man)' Makefile.in # For debugging.

mkdir build
cd build
../configure FOO=true --prefix="$(pwd)/_inst"
$EGREP 'MANS|\.([123456789]|man)' Makefile # For debugging.
touch foo.1 6.man
$MAKE install
$MAKE test1

cd ..
./configure FOO=false --prefix="$(pwd)/_inst"
$EGREP 'MANS|\.([123456789]|man)' Makefile # For debugging.
touch bar.2 baz.1 zap.5
$MAKE install
$MAKE test2

:
