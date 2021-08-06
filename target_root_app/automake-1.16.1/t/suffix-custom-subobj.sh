#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Tests that Automake understands suffix rules with subdir objects.
# Reported by John Ratliff.

required=cc
. test-init.sh

cat >>configure.ac <<EOF
AC_PROG_CC
AC_OUTPUT
EOF

cat >Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
SUFFIXES = .baz .o
# We fake here:
.baz.o:
## Account for VPATH issues on weaker make implementations.
	cp `test -f '$<' || echo $(srcdir)/`$< $@

bin_PROGRAMS = foo
foo_SOURCES = foo.c sub/bar.baz

.PHONY: test-fake test-real
test-fake:
	echo $(foo_OBJECTS) | grep '^foo\.quux sub/bar\.quux$$'
test-real:
	echo $(foo_OBJECTS) | grep '^foo\.$(OBJEXT) sub/bar\.$(OBJEXT)$$'
END

mkdir sub
: > sub/bar.baz
: > foo.c

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

run_make OBJEXT=quux test-fake
$MAKE test-real

:
