#! /bin/sh
# Copyright (C) 2008-2018 Free Software Foundation, Inc.
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

# Make sure nobase_* works for python files.

required=python
. test-init.sh

cat >>configure.ac <<EOF
AM_PATH_PYTHON
AC_OUTPUT
EOF

cat > Makefile.am <<'END'
mydir=$(prefix)/my
my_PYTHON = one.py sub/base.py
nobase_my_PYTHON = two.py sub/nobase.py
END

test_install()
{
  $MAKE install-data
  find inst -print # For debugging.
  py_installed inst/my/one.py
  py_installed inst/my/one.pyc
  py_installed inst/my/two.py
  py_installed inst/my/two.pyc
  py_installed inst/my/base.py
  py_installed inst/my/base.pyc
  py_installed inst/my/sub/nobase.py
  py_installed inst/my/sub/nobase.pyc
  py_installed --not inst/my/nobase.py
  py_installed --not inst/my/nobase.pyc
}

test_uninstall()
{
  $MAKE uninstall
  test -d inst/my
  ! find inst/my -type f -print | grep .
}

mkdir sub

for file in one.py sub/base.py two.py sub/nobase.py; do
  echo 'def one(): return 1' >$file
done

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure --prefix "$(pwd)/inst" --program-prefix=p

$MAKE
test_install
test_uninstall

$MAKE install-strip
test_uninstall

# Likewise, in a VPATH build.

$MAKE distclean
mkdir build
cd build
../configure --prefix "$(pwd)/inst" --program-prefix=p
test_install
test_uninstall

:
