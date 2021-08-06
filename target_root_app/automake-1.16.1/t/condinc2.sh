#! /bin/sh
# Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

# Another test of conditional include statements.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([TOBE], [false])
END

cat > Makefile.am << 'END'
if TOBE
include adjunct
endif
END

cat > adjunct << 'END'
target: dependency
	rule
endif
END

$ACLOCAL
AUTOMAKE_fails
grep 'adjunct:3: error: too many conditionals closed' stderr

cat > adjunct << 'END'
if TOBE
target: dependency
	rule
END

AUTOMAKE_fails
grep 'unterminated conditionals' stderr

cat > adjunct << 'END'
if TOBE
target: dependency
	rule
endif
END

$AUTOMAKE

:
