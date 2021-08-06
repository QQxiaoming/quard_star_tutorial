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

# Compiling .el files that requires each other in a VPATH build.
# See automake bug#11806.

required=emacs
. test-init.sh

cat >> configure.ac << 'END'
AM_PATH_LISPDIR
AC_OUTPUT
END

cat > Makefile.am << 'END'
lisp_LISP = foo.el
lisp_DATA = bar.el
END

echo "(require 'bar)" > foo.el
echo "(provide 'bar)" > bar.el

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

mkdir build
cd build
../configure
$MAKE
test -f foo.elc
cd ..

./configure
$MAKE
test -f foo.elc

:
