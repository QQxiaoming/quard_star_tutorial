#!/bin/sh
nfig
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

# Ensure an error with inconsistent state of conditionals in configure.ac.
# This shouldn't happen with user input, as _AM_COND_* are not documented,
# but better to be safe.

. test-init.sh

cat >>configure.ac <<'END'
AM_CONDITIONAL([COND], [:])
# The next line is needed so that cond-if.m4 is pulled in.
AM_COND_IF([COND])
_AM_COND_IF([COND])
AC_OUTPUT
END

edit_configure_ac ()
{
  sed "$@" < configure.ac >configure.tmp
  mv -f configure.tmp configure.ac
  rm -rf autom4te*.cache
}

: >Makefile.am

$ACLOCAL
AUTOMAKE_fails
grep '^configure\.ac:8:.* condition stack' stderr

edit_configure_ac 's/_AM_COND_IF/_AM_COND_ELSE/'
AUTOMAKE_fails
grep '^configure\.ac:7:.* else without if' stderr

edit_configure_ac 's/_AM_COND_ELSE/_AM_COND_ENDIF/'
AUTOMAKE_fails
grep '^configure\.ac:7:.* endif without if' stderr

edit_configure_ac 's/\(_AM_COND_ENDIF\).*/_AM_COND_IF\
_AM_COND_ENDIF/'
AUTOMAKE_fails
grep '^configure\.ac:7:.* not enough arguments.* _AM_COND_IF' stderr
grep '^configure\.ac:8:.* not enough arguments.* _AM_COND_ENDIF' stderr
test 2 -eq $($FGREP -c 'not enough arguments' stderr)

:
