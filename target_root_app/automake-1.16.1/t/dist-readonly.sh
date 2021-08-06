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

# Test that a readonly files are distributed as such, and not make
# writable while being copied in the $(distdir).

# This test expect the user to be unable to write on files lacking
# write permissions -- so it won't work if the user is 'root'.
required='non-root cc'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo
foo_SOURCES = foo.c
EXTRA_DIST = bar.txt
check-local: test
.PHONY: test
test:
	test -f $(srcdir)/foo.c && test ! -w $(srcdir)/foo.c
	if (echo x > $(srcdir)/foo.c); then exit 1; else :; fi
	grep 'main (void)' $(srcdir)/foo.c
	test -f $(srcdir)/bar.txt && test ! -w $(srcdir)/bar.txt
	if (echo x > $(srcdir)/bar.txt); then exit 1; else :; fi
	grep 'To be, or not to be' $(srcdir)/bar.txt
END

echo 'int main (void) { return 0; }' > foo.c
echo To be, or not to be ... > bar.txt
chmod a-w foo.c bar.txt

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE distdir
ls -l $distdir # For debugging.
test -f foo.c && test ! -w foo.c || exit 1
(echo x > foo.c) && exit 1
test -f bar.txt && test ! -w bar.txt || exit 1
(echo x > bar.txt) && exit 1
$MAKE distcheck

:
