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

# Check that no 'distcheck-hook' target is created automatically by
# Automake, and that a used-defined 'distcheck-hook' is *not* honored
# in a subpackage Makefile.am.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_SUBDIRS([subpkg])
AC_OUTPUT
END

mkdir subpkg

cat > subpkg/configure.ac << 'END'
AC_INIT([subpkg], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
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

cat > subpkg/Makefile.am <<'END'
check-local:
	ls -l $(srcdir)
	test ! -r $(srcdir)/dc-hook-has-run
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF
cd subpkg
$ACLOCAL
$AUTOMAKE
$AUTOCONF
cd ..

# For debugging.
$FGREP 'distcheck-hook' Makefile.in subpkg/Makefile.in

$FGREP 'distcheck-hook' subpkg/Makefile.in && exit 1
$FGREP '$(MAKE) $(AM_MAKEFLAGS) distcheck-hook' Makefile.in
grep '^distcheck-hook:' Makefile.in

./configure

$MAKE
$MAKE check && exit 1
cd subpkg
$MAKE check
cd ..
$MAKE distcheck

:
