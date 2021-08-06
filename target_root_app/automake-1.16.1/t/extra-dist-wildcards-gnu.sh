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

# Check for more complex usage of wildcards in EXTRA_DIST.
# Suggested by observations from Braden McDaniel.

required=GNUmake
. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am <<'END'
AUTOMAKE_OPTIONS = -Wno-portability

EXTRA_DIST = $(wildcard [!c-z].t d.t [ab].dir foo.* *zardoz*)

.PHONY: prereq
prereq:
	echo a > a.t
	echo b > b.t
	echo c > c.t
	echo d > d.t
	echo m > m.t
	echo z > z.t
	mkdir a.dir b.dir c.dir
	echo a1 > a.dir/f1
	echo a2 > a.dir/f2
	echo bb > b.dir/f
	echo cc > c.dir/x
	echo 0 > foo
	echo 1 > foo.x
	echo 2 > foo.bar
	echo foo > _zardoz_

.PHONY: test
test: distdir
	ls -l $(distdir) $(distdir)/*.dir ;: For debugging.
	diff a.t $(distdir)/a.t
	diff b.t $(distdir)/b.t
	test ! -r $(distdir)/c.t
	diff d.t $(distdir)/d.t
	test ! -r $(distdir)/m.t
	test ! -r $(distdir)/z.t
	diff a.dir/f1 $(distdir)/a.dir/f1
	diff a.dir/f2 $(distdir)/a.dir/f2
	diff b.dir/f $(distdir)/b.dir/f
	test ! -r $(distdir)/c.dir
	diff foo.x $(distdir)/foo.x
	diff foo.bar $(distdir)/foo.bar
	test ! -r $(distdir)/foo
	diff _zardoz_ $(distdir)/_zardoz_
check-local:
	ls -l . *.dir ;: For debugging.
	test -f $(srcdir)/a.t
	test -f $(srcdir)/b.t
	test ! -r $(srcdir)/c.t
	test -f $(srcdir)/d.t
	test ! -r $(srcdir)/m.t
	test ! -r $(srcdir)/z.t
	test -f $(srcdir)/a.dir/f1
	test -f $(srcdir)/a.dir/f2
	test -f $(srcdir)/b.dir/f
	test ! -r $(srcdir)/c.dir
	test -f $(srcdir)/foo.x
	test -f $(srcdir)/foo.bar
	test ! -r $(srcdir)/foo
	test -f $(srcdir)/_zardoz_
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure
$MAKE prereq
ls -l . *.dir # For debugging.
$MAKE test
$MAKE distcheck

:
