#! /bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

# Test for bug reported by Nyul Laszlo.  When using the ":" syntax in
# AC_OUTPUT, Automake fails to find the correct file.

. test-init.sh

cat >> configure.ac <<'END'
AC_CONFIG_FILES([foo.h:foo.hin])
AC_OUTPUT
END

: > foo.hin

cat > Makefile.am <<'END'
.PHONY: test
test: distdir
	ls -l . $(distdir)
	test -f foo.h
	test -f $(distdir)/foo.hin
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure

$MAKE test
$MAKE distcheck

:
