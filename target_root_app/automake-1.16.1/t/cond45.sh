#!/bin/sh
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

# Check that redefinitions of AC_SUBST'ed AM_SUBST_NOTMAKE'd variables
# are not diagnosed.  See 'cond24.sh'.

. test-init.sh

cat >>configure.ac <<EOF
AC_SUBST([foo], [bar])
AM_SUBST_NOTMAKE([foo])
AM_CONDITIONAL([COND], [true])
AC_OUTPUT
EOF

cat >Makefile.am <<EOF
if COND
foo = baz
endif
EOF

$ACLOCAL
AUTOMAKE_run
grep 'foo was already defined' stderr && exit 1
grep '^foo =' Makefile.in && exit 1
exit 0
