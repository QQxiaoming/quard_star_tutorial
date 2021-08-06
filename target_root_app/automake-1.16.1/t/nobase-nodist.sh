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

# Interaction of 'nobase_' and 'nodist_' prefixes.

. test-init.sh

# Simulate no emacs and no python (it shouldn't be needed in this test).
EMACS="no" PYTHON=":"; export EMCAS PYTHON

cat >> configure.ac << 'END'
AM_PATH_LISPDIR
AM_PATH_PYTHON([], [], [:])
# Simulate no javac (it shouldn't be needed in this test).
AC_SUBST([JAVAC], [false])
AC_OUTPUT
END

cat > Makefile.am << 'END'
nobase_nodist_noinst_PYTHON = baz1.py sub/baz2.py
# Lisp and Java sources are not distributed by default, so try both
# with and without the 'nodist_' prefix, for more coverage.
nobase_noinst_LISP = foo1.el sub/foo2.el
nobase_nodist_noinst_LISP = foo3.el sub/foo4.el
nobase_noinst_JAVA = bar1.java sub/bar2.java
nobase_nodist_noinst_JAVA = bar3.java sub/bar4.java
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
./configure
$MAKE distdir

ls -l $distdir $distdir/* # For debugging.
test ! -e $distdir/foo1.el
test ! -e $distdir/sub/foo2.el
test ! -e $distdir/foo3.el
test ! -e $distdir/sub/foo4.el
test ! -e $distdir/bar1.java
test ! -e $distdir/sub/bar2.java
test ! -e $distdir/bar3.java
test ! -e $distdir/sub/bar4.java
test ! -e $distdir/baz1.py
test ! -e $distdir/sub/baz2.py

:
