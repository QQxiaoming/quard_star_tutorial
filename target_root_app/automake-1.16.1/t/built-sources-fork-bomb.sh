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

# Due to how the handling of $(BUILT_SOURCES) is implemented in Automake-NG,
# a recursive make call in the recipe of any $(BUILT_SOURCES) (or of any of
# its prerequisites) might cause an infinite recursion (complete with fork
# bomb, yuck) if not handled correctly.  Verify that this doesn't happen.
# For more background, see:
# <https://lists.gnu.org/archive/html/help-smalltalk/2012-08/msg00027.html>
# <https://lists.gnu.org/archive/html/automake-patches/2012-08/msg00052.html>
# Backported to improve coverage of mainline Automake.

required=GNUmake
. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am << 'END'
BUILT_SOURCES = foo
.PHONY: build-foo
build-foo:
	echo OK > foo
foo:
	$(MAKE) build-foo

# If the bug is still present, we want this test to fail, not to actually
# go fork bomb and potentially crash the user machine.  Take care of that.

is_too_deep := $(shell test $(MAKELEVEL) -lt 10 && echo no)

## Extra indentation here required to avoid confusing Automake.
 ifeq ($(is_too_deep),no)
   # All is ok.
 else
   $(error ::OOPS:: Recursion too deep, $(MAKELEVEL) levels)
 endif
END

$ACLOCAL
$AUTOMAKE -Wno-portability
$AUTOCONF

./configure

run_make -M -- -n foo
test ! -f foo
# Guard against possible infinite recursion.
$FGREP '::OOPS::' output && exit 1

run_make -M -- foo
# Guard against possible infinite recursion.
$FGREP '::OOPS::' output && exit 1

:
