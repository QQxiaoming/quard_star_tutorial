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

# Make sure we define conditional _DEPENDENCIES correctly.

. test-init.sh

cat >>configure.ac <<'EOF'
AC_SUBST([CC], [false])
AC_SUBST([OBJEXT], [o])
AM_CONDITIONAL([C1], [test -z "$two"])
AM_CONDITIONAL([C2], [test -n "$two"])
AC_OUTPUT
EOF

cat > Makefile.am <<'EOF'
bin_PROGRAMS = a
a_LDADD = c0.o -L/some/where
if C1
a_LDADD += c1.o -llibfoo.a
endif
if C2
a_LDADD += c2.o -dlopen c3.la
endif

.PHONY: test1 test2
test1:
	is $(a_DEPENDENCIES) == c0.o c1.o
test2:
	is $(a_DEPENDENCIES) == c0.o c2.o c3.la
EOF

$ACLOCAL
$AUTOCONF
$AUTOMAKE --ignore-deps

./configure
$MAKE test1

./configure two=yes
$MAKE test2

:
