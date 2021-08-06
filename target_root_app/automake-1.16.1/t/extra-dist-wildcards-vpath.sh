#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Check that wildcards in elements of EXTRA_DIST are honoured when
# $srcdir != $builddir, if properly declared.
# Suggested by observations from Braden McDaniel.

required=GNUmake
. test-init.sh

echo AC_OUTPUT >> configure.ac


cat > Makefile.am <<'END'
EXTRA_DIST = *.foo $(srcdir)/*.foo $(builddir)/*.bar $(srcdir)/*.bar

.PHONY: test
test: distdir
	ls -l $(srcdir) $(builddir) $(distdir)
	diff $(srcdir)/a.foo $(distdir)/a.foo
	diff $(srcdir)/b.bar $(distdir)/b.bar
	diff $(builddir)/c.foo $(distdir)/c.foo
	diff $(builddir)/d.bar $(distdir)/d.bar
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

echo aaa > a.foo
echo bbb > b.bar
mkdir build
echo ccc > build/c.foo
echo ddd > build/d.bar

cd build
../configure
$MAKE test

:
