#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Test distribution of *_PYTHON files.

# This test does not require python.
. test-init.sh

cat >> configure.ac << 'END'
AM_PATH_PYTHON([], [:], [:])
AC_OUTPUT
END

cat > Makefile.am << 'END'
python_PYTHON = python.py
dist_python_PYTHON = python-dist.py
nodist_python_PYTHON = python-nodist.py

pkgpython_PYTHON = pkgpython.py
dist_pkgpython_PYTHON = pkgpython-dist.py
nodist_pkgpython_PYTHON = pkgpython-nodist.py

mydir = $(prefix)/my
my_PYTHON = my.py
dist_my_PYTHON = my-dist.py
nodist_my_PYTHON = my-nodist.py

my.py my-dist.py my-nodist.py:
	: > $@

.PHONY: disttest
disttest: distdir
	ls -l $(distdir)
	test -f $(distdir)/python.py
	test -f $(distdir)/python-dist.py
	test ! -f $(distdir)/python-nodist.py
	test -f $(distdir)/pkgpython.py
	test -f $(distdir)/pkgpython-dist.py
	test ! -f $(distdir)/pkgpython-nodist.py
	test -f $(distdir)/my.py
	test -f $(distdir)/my-dist.py
	test ! -f $(distdir)/my-nodist.py
END

: > python.py
: > python-dist.py
: > python-nodist.py

: > pkgpython.py
: > pkgpython-dist.py
: > pkgpython-nodist.py

: > py-compile

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

$MAKE disttest
# It's not distributed, hence it shouldn't be needlessly generated.
test ! -e my-nodist.py

:
