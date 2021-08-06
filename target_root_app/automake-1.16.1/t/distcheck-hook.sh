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

# Check basic support for distcheck-hook.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am <<'END'
distcheck-hook:
	ls -l $(distdir)
	chmod u+w $(distdir)
	: > $(distdir)/dc-hook-has-run
	chmod a-w $(distdir)
check-local:
	ls -l $(srcdir)
	test -f $(srcdir)/dc-hook-has-run
END

$ACLOCAL
$AUTOMAKE
$FGREP 'distcheck-hook' Makefile.in
$FGREP '$(MAKE) $(AM_MAKEFLAGS) distcheck-hook' Makefile.in
grep '^distcheck-hook:' Makefile.in

$AUTOCONF
./configure

$MAKE
$MAKE check && exit 1
$MAKE distdir
test -f $distdir/dc-hook-has-run && exit 1
$MAKE distcheck

:
