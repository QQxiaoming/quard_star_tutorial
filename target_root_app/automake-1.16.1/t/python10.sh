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

# Test _PYTHON with conditionals.

required=python
. test-init.sh

cat >>configure.ac <<'EOF'
AM_PATH_PYTHON
AM_CONDITIONAL([ONE], [test "x$one" = x1])
AC_OUTPUT
EOF

cat > Makefile.am <<'END'
if ONE
mydir=$(prefix)/my
my_PYTHON = one.py
else
yourdir=$(prefix)/your
your_PYTHON = two.py
endif

one.py:
	echo 'def one(): return 1' >$@
two.py:
	echo 'def two(): return 1' >$@

.PHONY: disttest
disttest: distdir
	ls -l $(distdir)
	test -f $(distdir)/one.py
	test -f $(distdir)/two.py
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

inst=inst_
mkdir inst_ build_
cd build_

cwd=$(pwd) || fatal_ "getting current working directory"

../configure --prefix="$cwd/$inst" one=0
$MAKE install
test -f            "$inst/your/two.py"
py_installed       "$inst/your/two.pyc"
py_installed       "$inst/your/two.pyo"
py_installed --not "$inst/my/one.py"
py_installed --not "$inst/my/one.pyc"
py_installed --not "$inst/my/one.pyo"
$MAKE uninstall
py_installed --not "$inst/your/two.py"
py_installed --not "$inst/your/two.pyc"
py_installed --not "$inst/your/two.pyo"

../configure --prefix=$cwd/"$inst" one=1
$MAKE install
py_installed --not "$inst/your/two.py"
py_installed --not "$inst/your/two.pyc"
py_installed --not "$inst/your/two.pyo"
test -f            "$inst/my/one.py"
py_installed       "$inst/my/one.pyc"
py_installed       "$inst/my/one.pyo"
$MAKE uninstall
py_installed --not "$inst/my/one.py"
py_installed --not "$inst/my/one.pyc"
py_installed --not "$inst/my/one.pyo"

$MAKE disttest

:
