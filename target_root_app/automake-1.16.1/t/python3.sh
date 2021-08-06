#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Make sure we install built python files.

required=python
. test-init.sh

cat >>configure.ac <<EOF
AM_PATH_PYTHON
AC_OUTPUT
EOF

cat > Makefile.am <<'END'
mydir=$(prefix)/my
my_PYTHON = one.py

one.py:
	echo 'def one(): return 1' >$@
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

mkdir build
cd build
../configure --prefix="$(pwd)/inst"
$MAKE install
py_installed inst/my/one.py
py_installed inst/my/one.pyc
py_installed inst/my/one.pyo

:
