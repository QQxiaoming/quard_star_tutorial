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

# Check that wildcards in EXTRA_DIST are honoured.
# Suggested by observations from Braden McDaniel.
# See also sister test 'extra-dist-wildcards-gnu.sh', that checks a
# similar usage with the involvement of the $(wildcard) GNU make builtin.

required=GNUmake
. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am <<'END'
EXTRA_DIST = *.foo
.PHONY: test
test: distdir
	diff a.foo $(distdir)/a.foo
	diff b.foo $(distdir)/b.foo
	test ! -r $(distdir)/c.bar
check-local:
	test -f $(srcdir)/a.foo
	test -f $(srcdir)/b.foo
	test ! -r $(srcdir)/c.bar
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

echo aaa > a.foo
echo bbb > b.foo
echo ccc > c.bar
./configure
$MAKE test

:
