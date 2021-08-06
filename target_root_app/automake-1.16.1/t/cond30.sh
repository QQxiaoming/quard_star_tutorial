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

# For PR/352: make sure we support bin_PROGRAMS being defined conditionally.

. test-init.sh

cat >>configure.ac <<'EOF'
AM_CONDITIONAL([C1], [test -z "$two"])
AM_CONDITIONAL([C2], [test -n "$two"])
AC_OUTPUT
EOF

cat > Makefile.am <<'EOF'
AUTOMAKE_OPTIONS = no-dependencies
CC = false
EXEEXT = .foo

if C1
bin_PROGRAMS = a
endif
if C2
bin_PROGRAMS = b $(undefined)
endif

.PHONY: test-a test-b
test-a:
	test a.foo = $(bin_PROGRAMS)
test-b:
	test b.foo = $(bin_PROGRAMS)
EOF

$ACLOCAL
$AUTOCONF
$AUTOMAKE

$FGREP 'a_SOURCES = a.c' Makefile.in
$FGREP 'b_SOURCES = b.c' Makefile.in

./configure
$MAKE test-a

./configure two=yes
$MAKE test-b

:
