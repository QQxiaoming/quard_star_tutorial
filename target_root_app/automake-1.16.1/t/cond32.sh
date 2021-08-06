#!/bin/sh
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

# Make sure the user can override a conditional _DEPENDENCIES.

. test-init.sh

cat >>configure.ac <<'EOF'
AM_CONDITIONAL([C1], [test -z "$two"])
AM_CONDITIONAL([C2], [test -n "$two"])
AM_CONDITIONAL([C3], [test -z "$three"])
# We define CC in Makefile.am, but OBJEXT here.
OBJEXT=o; AC_SUBST([OBJEXT])
AC_SUBST([MYSUB], ["foo.$OBJEXT"])
AC_OUTPUT
EOF

cat >>Makefile.am <<'EOF'
AUTOMAKE_OPTIONS = no-dependencies
CC = :

bin_PROGRAMS = a

if C1
a_LDADD = $(MYSUB)
a_DEPENDENCIES = $(MYSUB) nonsense.a
# Note that 'nonsense.a' is there just to make sure Automake insn't
# using some self computed a_DEPENDENCIES variable.
endif

if C2
if C3
BAR = bar.o
else
BAR = baz.o
endif
a_LDADD = $(BAR)
endif

test:
	is $(exp) == $(a_DEPENDENCIES)
.PHONY: test
EOF

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE test exp='foo.o nonsense.a'

./configure two=yes three=
$MAKE test exp='bar.o'

./configure two=yes three=yes
$MAKE test exp='baz.o'

:
