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

# Check that 'aclocal.m4' is not automatically distributed if not
# required to build 'configure'.  This is *really* a corner-case
# check, and the behaviour it checks is not documented either, so
# if that behaviour is deliberately changed in the future, just
# remove this test.
# Related to automake bug#7819.

. test-init.sh

{ echo 'm4_include([defs.m4])'
  cat configure.ac
  echo 'AC_OUTPUT'
} > t
mv -f t configure.ac

cat > Makefile.am <<'END'
.PHONY: test
test: distdir
	ls -l $(distdir)
	test ! -f $(distdir)/aclocal.m4
	echo $(DISTFILES) | grep 'aclocal\.m4' && exit 1; :
	echo $(DIST_COMMON) | grep 'aclocal\.m4' && exit 1; :
check-local: test
END

: > defs.m4
$ACLOCAL
mv -f aclocal.m4 defs.m4

$AUTOMAKE
$AUTOCONF

./configure
$MAKE test
$MAKE distcheck

:
