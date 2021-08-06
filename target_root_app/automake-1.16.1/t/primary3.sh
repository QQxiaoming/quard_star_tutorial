#! /bin/sh
# Copyright (C) 2004-2018 Free Software Foundation, Inc.
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

# Make sure we can build programs ending in '.la'

required=cc
. test-init.sh

cat >>configure.ac <<EOF
AC_PROG_CC
AC_OUTPUT
EOF

cat > Makefile.am << 'END'
bin_PROGRAMS = foo.la
foo_la_SOURCES = foo.c
.PHONY: test
test: all
	test -f foo.la$(EXEEXT)
END

echo 'int main () { return 0; }' > foo.c

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure
$MAKE test

:
