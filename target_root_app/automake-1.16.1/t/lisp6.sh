#! /bin/sh
# Copyright (C) 2004-2018 Free Software Foundation, Inc.
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

# Test for conditional _LISP.

required=emacs
. test-init.sh

cat > Makefile.am << 'EOF'
dist_lisp_LISP = am-one.el
if WANT_TWO
  dist_lisp_LISP += am-two.el
endif
dist_noinst_LISP = am-three.el

dist-test: distdir
	test -f $(distdir)/am-one.el
	test -f $(distdir)/am-two.el
	test -f $(distdir)/am-three.el
EOF

cat >> configure.ac << 'EOF'
AM_CONDITIONAL([WANT_TWO], [test -n "$want_two"])
AM_PATH_LISPDIR
AC_OUTPUT
EOF

# Avoid possible spurious influences from the environment.
unset want_two

echo "(provide 'am-one)" > am-one.el
echo "(require 'am-one)" > am-two.el
echo "(require 'am-one)" > am-three.el

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

cwd=$(pwd) || fatal_ "getting current working directory"

./configure --with-lispdir="$cwd/lisp"

$MAKE
test -f am-one.elc
test ! -e am-two.elc
test -f am-three.elc

$MAKE install
test -f lisp/am-one.el
test -f lisp/am-one.elc
test ! -e lisp/am-two.el
test ! -e lisp/am-two.elc
test ! -e lisp/am-three.el
test ! -e lisp/am-three.elc

$MAKE dist-test

$MAKE distclean
test ! -e am-one.elc
test ! -e am-two.elc
test ! -e am-three.elc

./configure --with-lispdir="$cwd/lisp" want_two=1

$MAKE
test -f am-one.elc
test -f am-two.elc
test -f am-three.elc

# Let's mutilate the source tree, to check the recover rule.
rm -f am-*.elc
$MAKE
test -f am-one.elc
test -f am-two.elc
test -f am-three.elc

$MAKE install
test -f lisp/am-one.el
test -f lisp/am-one.elc
test -f lisp/am-two.el
test -f lisp/am-two.elc
test ! -e lisp/am-three.el
test ! -e lisp/am-three.elc

$MAKE dist-test

$MAKE distclean
test ! -e am-one.elc
test ! -e am-two.elc
test ! -e am-three.elc

:
