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

# Check the documented limitation of the Automake's Makefile parser w.r.t.
# use of TAB characters; see Section "General Operation" in the Automake
# manual, and automake bug#8360.
# If you cause some parts of this test to fail, chances are that you've
# improved the Automake parser ;-)

. test-init.sh

cat > Makefile.am <<END
.PHONY: test
test: all check

fail:
${tab}@echo "'\$@ recipe executed'"; exit 1

## This won't be recognized as a target+recipe by Automake.
all-local${tab}:
${tab}@exit 1

## This won't be recognized as a target+rdependency by Automake.
all-local${tab}: fail

just_to_separate: dummy deps

## This won't be recognized as a variable assignment by Automake.
${tab}bin_PROGRAMS = foo
END

echo AC_OUTPUT >> configure.ac

$ACLOCAL
$AUTOMAKE

$FGREP '$(EXEEEXT)' Makefile.in && exit 1
grep 'all:.*all-local' Makefile.in && exit 1
grep "^${tab}bin_PROGRAMS = foo" Makefile.in

$AUTOCONF
./configure

$MAKE test

:
