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

# Emacs lisp files in subdirectories.

required=emacs
. test-init.sh

cat >> configure.ac << 'END'
AM_PATH_LISPDIR
AC_OUTPUT
END

cat > Makefile.am << 'END'
nobase_lisp_LISP = sub/am-one.el sub/am-two.el sub/am-three.el
nobase_dist_lisp_LISP = sub/am-three.el
dist_noinst_LISP = sub/am-four.el
EXTRA_DIST = sub/am-one.el sub/am-two.el
END

mkdir sub
echo "(require 'am-two) (provide 'am-one)"      > sub/am-one.el
echo "(require 'am-three) (provide 'am-two)"    > sub/am-two.el
echo "(provide 'am-three)"                      > sub/am-three.el
echo "(require 'am-one) (require 'am-two)"      > sub/am-four.el

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure --prefix="$(pwd)/inst" \
            --with-lispdir="$(pwd)/inst/elisp" \

$MAKE
# No byte-compiled elisp files in the top-level directory.
test "$(echo *.elc)" = '*.elc'
test -f sub/am-one.elc
test -f sub/am-two.elc
test -f sub/am-three.elc
test -f sub/am-four.elc
$MAKE clean
test ! -e sub/am-one.elc
test ! -e sub/am-two.elc
test ! -e sub/am-three.elc
test ! -e sub/am-four.elc

# Check installation.

sort > exp << 'END'
inst/elisp/sub/am-one.elc
inst/elisp/sub/am-two.elc
inst/elisp/sub/am-three.elc
END

$MAKE install
find inst # For debugging.
find inst -name '*.elc' > lst || { cat lst; exit 1; }
sort lst > got

cat exp
cat got
diff exp got

# Also check VPATH builds and uninstall completeness.
$MAKE distcheck

:
