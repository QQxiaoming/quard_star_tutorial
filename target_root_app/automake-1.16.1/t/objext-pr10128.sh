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

# Test for automake bug#10128: $(OBJEXT) redefinition causes
# $(foo_OBJECTS) to be defined as empty.

. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am <<'END'
LINK = echo >$@ Linked $@ from
OBJEXT = fasl
EXEEXT =

noinst_PROGRAMS = foo zardoz
foo_SOURCES = foo.lisp
zardoz_SOURCES = mu1.lisp mu2.lisp

## Un-commenting this is enough to make the test pass.  Weird!
##.lisp.o:

.lisp.fasl:
	touch $@

.PHONY: test
test:
	is $(foo_OBJECTS) == foo.fasl
	is $(zardoz_OBJECTS) == mu1.fasl mu2.fasl
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure
$MAKE test
touch foo.lisp mu1.lisp mu2.lisp
$MAKE all
cat foo
cat zardoz
test "$(cat foo)"    = "Linked foo from foo.fasl"
test "$(cat zardoz)" = "Linked zardoz from mu1.fasl mu2.fasl"

:
