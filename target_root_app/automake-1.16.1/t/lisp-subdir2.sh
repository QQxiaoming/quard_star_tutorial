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

# Compiling elisp files in different subdirectories.

required=emacs
. test-init.sh

cat >> configure.ac << 'END'
AM_PATH_LISPDIR
AC_OUTPUT
END

cat > Makefile.am << 'END'
dist_lisp_LISP = am-one.el
nodist_lisp_LISP = sub/am-two.el
sub/am-two.el:
	mkdir sub
	echo "(provide 'am-two)" > $@
DISTCLEANFILES = $(nodist_lisp_LISP)
dist_noinst_LISP = x/y/z/am-three.el

elc-test:
	test -f am-one.elc
	test -f sub/am-two.elc
	test -f x/y/z/am-three.elc
.PHONY: elc-test
check-local: elc-test
END

mkdir x x/y x/y/z
echo "(provide 'am-one)" > am-one.el
# sub/am-two.el is generated at make runtime
echo "(provide 'am-three)" > x/y/z/am-three.el

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure

$MAKE
$MAKE elc-test
$MAKE clean
test ! -e am-one.elc
test ! -e sub/am-two.elc
test ! -e x/y/z/am-three.elc
test -f am-one.el
test -f sub/am-two.el
test -f x/y/z/am-three.el

# Byte-compiling only a subset of the elisp files.
$MAKE am-one.elc sub/am-two.elc
test -f am-one.elc
test -f sub/am-two.elc
test ! -e x/y/z/am-three.elc
rm -f am-one.elc sub/am-two.elc
$MAKE x/y/z/am-three.elc
test ! -e am-one.elc
test ! -e sub/am-two.elc
test -f x/y/z/am-three.elc

$MAKE distcheck

:
