#! /bin/sh
# Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

# Ensure that "make dist" no longer fails when a distributed file
# depends on a file from the list of BUILT_SOURCES.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
BUILT_SOURCES = h.h
h.h:
	rm -f $@ $@-t
	printf '%s\n' '#define F "F"' > $@-t
	mv -f $@-t $@
CLEANFILES = h.h

EXTRA_DIST = gen
gen: foo
	./foo > $@-t && mv $@-t $@

bin_PROGRAMS = foo
foo_SOURCES = foo.c
END

cat > foo.c << 'END'
#include "h.h"
int main (void) { printf ("%s\n", F); return 0; }
END
chmod a-w foo.c

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE dist

:
