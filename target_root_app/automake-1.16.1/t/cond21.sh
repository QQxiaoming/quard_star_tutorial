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

# Check for use of = and += in different conditions.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([COND1], [true])
AM_CONDITIONAL([COND2], [true])
AM_CONDITIONAL([COND3], [true])
AC_OUTPUT
END

cat > Makefile.am << 'END'

FOO = foo
if COND1
  FOO += foo1
else
  FOO += foon1
endif
if COND2
  FOO += foo2
else
  FOO += foon2
endif
if COND1
  FOO += foo1b
else
  FOO += foon1b
endif

if COND1
if COND2
  BAR = bar12
else
  BAR = bar1n2
endif
else
  BAR = barn1
endif

BAR += bar

if COND3
  BAR += bar3
endif

.PHONY: test
test:
	@echo BAR: $(BAR) :BAR
	@echo FOO: $(FOO) :FOO
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
./configure
$MAKE test | $FGREP 'BAR: bar12 bar bar3 :BAR'
$MAKE test | $FGREP 'FOO: foo foo1 foo2 foo1b :FOO'

:
