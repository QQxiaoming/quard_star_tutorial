#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Make sure $(EXEEXT) is appended to programs and to tests that are
# programs, but not to @substitutions@.

# For gen-testsuite-part: ==> try-with-serial-tests <==
. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([COND], [test -n "$cond"])
AC_SUBST([programs], ['prg1$(EXEEXT) prg2$(EXEEXT)'])
AC_SUBST([CC], [whocares])
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = no-dependencies
EXEEXT = .x
if COND
  BAR = bar
  DEP = bar
  BAZE = baz$(EXEEXT)
  BAZ = baz $(DEP)
endif
bin_PROGRAMS = $(programs) @programs@ prg3 $(BAR) $(BAZE)
EXTRA_PROGRAMS = prg1 prg2 prg3
TESTS = prg1 prg3 prg4 $(BAZ)

.PHONY: test-cond test-nocond
test-nocond:
	is $(bin_PROGRAMS) == prg1.x prg2.x prg1.x prg2.x prg3.x
	is $(EXTRA_PROGRAMS) == prg1.x prg2.x prg3.x
	is $(TESTS) == prg1.x prg3.x prg4
test-cond:
	is $(bin_PROGRAMS) == prg1.x prg2.x prg1.x prg2.x prg3.x bar.x baz.x
	is $(EXTRA_PROGRAMS) == prg1.x prg2.x prg3.x
	is $(TESTS) == prg1.x prg3.x prg4 baz.x bar.x
	is $(BAR) $(BAZ) == bar baz bar
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing --copy

./configure
$MAKE test-nocond

./configure cond=yes
$MAKE test-cond

# Only two am__EXEEXT_* variables are needed here: one for BAR, and one
# BAZ.  The latter must use the former.
test 2 -eq $(grep -c '__EXEEXT_. =' Makefile.in)
grep 'am__EXEEXT_2 = .*am__EXEEXT_1' Makefile.in

:
