#! /bin/sh
# Copyright (C) 2005-2018 Free Software Foundation, Inc.
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

# Make sure that lisp_LISP also works when emacs is not installed.

. test-init.sh

cat > Makefile.am << 'EOF'
dist_lisp_LISP = am-one.el am-two.el am-three.el
EOF

cat >> configure.ac << 'EOF'
AM_PATH_LISPDIR
EMACS=no # Simulate no emacs.
AC_OUTPUT
EOF

echo "(require 'am-two)" > am-one.el
echo "(require 'am-three) (provide 'am-two)" > am-two.el
echo "(provide 'am-three)" > am-three.el

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing
./configure

run_make -O
test ! -e am-one.elc
test ! -e am-two.elc
test ! -e am-three.elc

$MAKE distcheck

:
