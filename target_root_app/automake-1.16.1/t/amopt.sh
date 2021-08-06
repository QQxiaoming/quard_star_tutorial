#! /bin/sh
# Copyright (C) 2006-2018 Free Software Foundation, Inc.
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

# Make Automake diagnose a conditional AUTOMAKE_OPTIONS.
# Report from Bas Wijnen.

. test-init.sh

cat >>configure.ac <<END
AM_CONDITIONAL([COND], [true])
END

mkdir sub

cat >Makefile.am <<END
if COND
AUTOMAKE_OPTIONS = -Wall
endif
END

$ACLOCAL
AUTOMAKE_fails
grep '^Makefile\.am:2.*AUTOMAKE_OPTIONS.*conditional' stderr

:
