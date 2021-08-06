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

# Test that compiling interdependent elisp files works.

required=emacs
. test-init.sh

cat > Makefile.am << 'EOF'
lisp_LISP = am-one.el am-two.el am-three.el
EXTRA_DIST = am-one.el am-two.el
am-three.el:
	echo "(provide 'am-three)" > $@
CLEANFILES = am-three.el
EOF

cat >> configure.ac << 'EOF'
AM_PATH_LISPDIR
AC_OUTPUT
EOF

echo "(require 'am-two)" > am-one.el
echo "(require 'am-three) (provide 'am-two)" > am-two.el
# am-three.el is a built source

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure --prefix="$(pwd)/_inst"

$MAKE

test -f am-one.elc
test -f am-two.elc
test -f am-three.elc

# Make sure we can recover from a deletion.
rm -f am-one.elc
$MAKE
test -f am-one.elc

# Test installation/deinstallation.

$MAKE install

find _inst # For debugging.

# Keep thin in sync with m4/lispdir.m4.
for dir in lib/emacs lib/xemacs share/emacs share/xemacs :; do
  if test $dir = :; then
    exit 1
  elif test -d _inst/$dir/site-lisp; then
    break
  fi
done

test -f _inst/$dir/site-lisp/am-one.el
test -f _inst/$dir/site-lisp/am-one.elc
test -f _inst/$dir/site-lisp/am-two.el
test -f _inst/$dir/site-lisp/am-two.elc
test -f _inst/$dir/site-lisp/am-three.el
test -f _inst/$dir/site-lisp/am-three.elc

$MAKE uninstall
find _inst | $EGREP '\.elc?$' && exit 1

# It should also work for VPATH-builds.
$MAKE distcheck

:
