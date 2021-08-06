#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Make sure setting ELCFILES= disable byte-compilation as documented.
# Report from Simon Josefsson.

required=emacs
. test-init.sh

cat > Makefile.am << 'EOF'
lisp_LISP = am-one.el am-two.el am-three.el
EXTRA_DIST = am-one.el am-two.el
ELCFILES=
am-three.el:
	echo "(provide 'am-three)" > $@
CLEANFILES = am-three.el

test:
	test ! -f am-one.elc
	test ! -f am-two.elc
	test ! -f am-three.elc

install-test: install
	test -f "$(lispdir)/am-one.el"
	test -f "$(lispdir)/am-two.el"
	test -f "$(lispdir)/am-three.el"
	test ! -f "$(lispdir)/am-one.elc"
	test ! -f "$(lispdir)/am-two.elc"
	test ! -f "$(lispdir)/am-three.elc"

not-installed:
	find "$(lispdir)" | grep '\.el$$' && exit 1; :
	find "$(lispdir)" | grep '\.elc$$' && exit 1; :
EOF

cat >> configure.ac << 'EOF'
AM_PATH_LISPDIR
AC_OUTPUT
EOF

echo "(require 'am-two)" > am-one.el
echo "(require 'am-three) (provide 'am-two)" > am-two.el
# am-tree.el is a built source

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

cwd=$(pwd) || fatal_ "getting current working directory"

./configure --prefix "$cwd"
$MAKE
$MAKE test
$MAKE install-test
$MAKE uninstall
$MAKE not-installed

# Fake the absence of emacs.
# *.el files should not be installed, but "make install" and
# "make uninstall" should continue to work.
./configure EMACS=no --prefix "$cwd"
$MAKE
$MAKE test
$MAKE install
$MAKE not-installed
$MAKE uninstall

:
