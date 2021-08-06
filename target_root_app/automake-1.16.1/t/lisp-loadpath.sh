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

# Emacs lisp files in both $(srcdir) and $(builddir) are found if
# required by other files.  Related to automake bug#11806.

required=emacs
. test-init.sh

cat >> configure.ac << 'END'
AM_PATH_LISPDIR
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_LISP = requirer.el
lisp_LISP = foo.el
lisp_DATA = bar.el
END

echo "(require 'foo) (require 'bar)" >> requirer.el
echo "(provide 'foo)" > foo.el
echo "(provide 'bar)" > bar.el

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

mkdir build
cd build
../configure
$MAKE
test -f requirer.elc
test -f foo.elc
test ! -e bar.elc

$MAKE clean
test ! -e requirer.elc
test ! -e foo.elc

# In the spirit of VPATH, stuff in the builddir is preferred to
# stuff in the srcdir.
echo "(provide" > ../foo.el  # Break it.
echo "defun)" > ../bar.el    # Likewise.
$MAKE && exit 1
$sleep
echo "(provide 'foo)" > foo.el
echo "(provide 'bar)" > bar.el
$MAKE
test -f requirer.elc
test -f foo.elc
test ! -e bar.elc

:
