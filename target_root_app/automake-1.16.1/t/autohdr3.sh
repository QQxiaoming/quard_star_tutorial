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

# Check rebuild rules for autoheader.

. test-init.sh

cat >>configure.ac <<EOF
m4_include([foo.m4])
AC_CONFIG_HEADERS([config.h:config.hin])
AC_OUTPUT
EOF

: > foo.m4

cat > Makefile.am <<'END'
.PHONY: test
check-local: test
test: all
	cat $(srcdir)/config.hin ;: For debugging.
	cat config.h             ;: Likewise.
	grep '#.*GREPME' $(srcdir)/config.hin
	grep '#.*define.*GREPME' config.h
END

$ACLOCAL
$AUTOCONF
$AUTOHEADER
$AUTOMAKE

./configure
$MAKE

$sleep
echo 'AC_DEFINE([GREPME], 1, [Doc for GREPME])' > foo.m4

$MAKE test
$MAKE distcheck

:
