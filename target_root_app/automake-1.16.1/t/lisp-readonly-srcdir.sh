#! /bin/sh
# Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

# Ensure that building elisp from a read-only srcdir works.

required=emacs
. test-init.sh

cat > Makefile.am << 'EOF'
lisp_LISP = am-one.el
EOF

cat >> configure.ac << 'EOF'
AM_PATH_LISPDIR
AC_OUTPUT
EOF

echo > am-one.el

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

mkdir sub
chmod a=rx .

cd sub
../configure
$MAKE

test -f am-one.elc

:
