#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Test of subdir make dist rules.
# From Robert Collins.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([generic/Makefile])
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = wish
wish_SOURCES = generic/a.c

.PHONY: test-distdir test-build
test-distdir: distdir
	ls -l $(distdir) $(distdir)/* ;: For debugging.
	test ! -r $(distdir)/a.c
	test -f $(distdir)/generic/a.c
test-build: all
	ls -l . generic ;: For debugging.
	test -f generic/a.$(OBJEXT)
	test -f wish$(EXEEXT)
	test ! -r a.$(OBJEXT)
	test ! -r a$(EXEEXT)
	test ! -r generic/wish$(EXEEXT)
	test ! -r generic/a$(EXEEXT)
END

mkdir generic
: > generic/Makefile.am

cat > generic/a.c << 'END'
int main (void)
{
  return 0;
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --include-deps --copy --add-missing

./configure

$MAKE test-distdir
$MAKE dist
$MAKE test-build

:
