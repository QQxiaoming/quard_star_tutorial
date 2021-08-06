#!/bin/sh
# Copyright (C) 2005-2018 Free Software Foundation, Inc.
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

# Check conditional variable ordering.
# Report from Ed Hartnett.

. test-init.sh

cat >>configure.ac <<'EOF'
AM_CONDITIONAL([CASE_A], :)
AM_CONDITIONAL([CASE_B], :)
AC_OUTPUT
EOF

cat > Makefile.am <<'EOF'
SUBDIRS = a
if CASE_A
SUBDIRS += b
endif
SUBDIRS += c
if CASE_A
SUBDIRS += d
if CASE_B
SUBDIRS += e
endif
SUBDIRS += f
endif
SUBDIRS += g
if CASE_B
SUBDIRS += h
endif
if CASE_B
SUBDIRS += iXYZ
SUBDIRS += jZYX
endif

.PHONY: test
test:
	is $(SUBDIRS) == a b c d e f g h iXYZ jZYX
EOF

mkdir a b c d e f g h iXYZ jZYX

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
# Make sure no extra variable was created for the last 3 items.
grep 'append.*=.* h iXYZ jZYX' Makefile
# Check good ordering.
$MAKE test

:
