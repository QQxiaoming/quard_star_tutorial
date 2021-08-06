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

# Ensure DESTDIR is not included in byte-compiled files.

required=python
. test-init.sh

cat >>configure.ac <<'EOF'
AM_PATH_PYTHON
AC_OUTPUT
EOF

cat >Makefile.am <<'END'
mydir = $(datadir)/my
my_PYTHON = my.py

my.py:
	echo 'def my(): return 1' >$@
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

destdir=$(pwd)/inst
mkdir inst build
cd build
../configure --prefix="/usr"
$MAKE install DESTDIR=$destdir

# Perfunctory test that the files were created.
test -f "$destdir/usr/share/my/my.py"
pyo=$(pyc_location -p "$destdir/usr/share/my/my.pyo")
pyc=$(pyc_location -p "$destdir/usr/share/my/my.pyc")

# If DESTDIR has made it into the byte compiled files, fail the test.
st=0; $FGREP "$destdir" "$pyc" "$pyo" || st=$?
test $st -eq 1

:
