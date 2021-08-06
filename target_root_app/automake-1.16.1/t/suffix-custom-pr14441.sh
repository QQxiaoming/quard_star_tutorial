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

# Test for bug#14441: the same user-defined suffix being transformed into
# different automake-known suffixes in different Makefiles was confusing
# Automake.

. test-init.sh

cat >>configure.ac <<EOF
AC_PROG_CC
AC_PROG_CXX
AC_CONFIG_FILES([sub/Makefile])
EOF

mkdir sub

cat > Makefile.am <<'END'
SUBDIRS = sub
bin_PROGRAMS = one
one_SOURCES = one.ext
.ext.cc:
	whatever
END

cat > sub/Makefile.am <<'END'
bin_PROGRAMS = two
two_SOURCES = two.ext
.ext.c:
	do something
END

$ACLOCAL
$AUTOMAKE -a

$FGREP '.c' Makefile.in sub/Makefile.in # For debugging.

grep '^\.cc\.o:' Makefile.in
LC_ALL=C $EGREP '\.c[^a-z]' Makefile.in && exit 1
grep '^\.c\.o:' sub/Makefile.in
$FGREP '.cc' sub/Makefile.in && exit 1

:
