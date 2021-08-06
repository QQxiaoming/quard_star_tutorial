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

# Test AM_COND_IF.

. test-init.sh

cat >>configure.ac <<'END'
AC_DEFUN([FOO],
	 [AC_CONFIG_FILES([$1])])

AM_CONDITIONAL([COND], [test "$cond" = yes])
# Next lines should not cause a shell syntax error.
AM_COND_IF([COND])
AM_COND_IF([COND],
	   [AC_SUBST([BAR])])
AM_COND_IF([COND],
	   [AC_CONFIG_FILES([file1])])

# Things should work even at a time when the shell expressions
# for the conditional are not valid any more.
ok=$cond1
AM_CONDITIONAL([COND1], [test "$ok" = yes])
ok=$cond2
AM_CONDITIONAL([COND2], [test "$ok" = yes])
ok=$cond3
AM_CONDITIONAL([COND3], [test "$ok" = yes])

AM_COND_IF([COND1],
	   [AM_COND_IF([COND2], [FOO([file2])],
		       [AM_COND_IF([COND3],
				   [FOO([file3])])])])

AC_OUTPUT
END

: >Makefile.am
: >file1.in
: >file2.in
: >file3.in

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure cond=yes cond1=yes cond2=no cond3=yes
test -f file1
test ! -e file2
test -f file3
rm -f file1 file3
$MAKE file1 file3
$MAKE file2 && exit 1
test -f file1
test ! -e file2
test -f file3
$MAKE distclean

./configure cond=no cond1=yes cond2=yes
test ! -e file1
test -f file2
test ! -e file3
rm -f file2
$MAKE file1 && exit 1
$MAKE file2
$MAKE file3 && exit 1
test ! -e file1
test -f file2
test ! -e file3

:
