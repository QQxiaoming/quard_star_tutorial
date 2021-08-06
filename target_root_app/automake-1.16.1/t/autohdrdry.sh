#!/bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Removal recovery rules for AC_CONFIG_HEADERS should not remove files
# with 'make -n'.

. test-init.sh

cat >>configure.ac <<'EOF'
AC_CONFIG_HEADERS([config.h])
AC_OUTPUT
EOF

: >Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOHEADER
$AUTOMAKE

./configure
$MAKE

rm -f config.h
$MAKE -n
test -f stamp-h1
test ! -e config.h

:
