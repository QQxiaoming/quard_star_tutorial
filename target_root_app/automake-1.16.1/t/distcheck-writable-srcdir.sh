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

# Check that the user abuse 'distcheck-hook' to ensure his packages
# is built by "make distcheck" with a writable srcdir.  This goes
# against the GNU Coding Standards, but some package authors might
# not care about them too much, and it's nice to support their use
# case too (since it doesn't cost us anything but this test).
# See automake bug#10878.

. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am <<'END'
EXTRA_DIST = old
distcheck-hook:
	find $(distdir) -exec chmod u+w '{}' ';'
check-local:
	test -f $(srcdir)/old
	test ! -f $(srcdir)/new
	test ! -r $(srcdir)/new
## Don't do this in real life, or I'll kill you.
	echo foo > $(srcdir)/old
	echo bar > $(srcdir)/new
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF
./configure
: > old
$MAKE distcheck

:
