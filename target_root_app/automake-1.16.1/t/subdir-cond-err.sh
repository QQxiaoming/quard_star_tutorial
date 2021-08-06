#! /bin/sh
# Copyright (C) 1997-2018 Free Software Foundation, Inc.
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

# Check SUBDIRS set based on conditionals.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([TEST], [true])
END

cat > Makefile.am << 'END'
if TEST
DIR = dir1
else
DIR = dir2
endif
SUBDIRS = $(DIR)
END

mkdir dir1

$ACLOCAL
AUTOMAKE_fails
grep '^Makefile\.am:4:.*dir2.*does not exist' stderr

:
