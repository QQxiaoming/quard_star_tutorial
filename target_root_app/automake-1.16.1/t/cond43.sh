#!/bin/sh
# Copyright (C) 2008-2018 Free Software Foundation, Inc.
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

# Ensure an error with underquoted usage of AM_COND_IF in configure.ac.

. test-init.sh

cat >>configure.ac <<'END'
AM_CONDITIONAL([COND1], [:])
AM_CONDITIONAL([COND2], [:])
AM_COND_IF([COND1],
           AM_COND_IF([COND2], [:])
)
AC_OUTPUT
END

: >Makefile.am

$ACLOCAL
AUTOMAKE_fails
$EGREP '^configure\.ac:7:.* missing m4 quoting.*macro depth 2( |$)' stderr

sed '/.AM_COND_IF/{
        s/^/[/
        s/$/]/
     }' < configure.ac > configure.tmp
mv -f configure.tmp configure.ac
rm -rf autom4te*.cache
$AUTOMAKE

:
