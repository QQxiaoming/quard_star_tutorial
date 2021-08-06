#! /bin/sh
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

# Test that conditional overrides are diagnosed.
# This test is still xfailing; see the FIXME in Automake::Rule::define.
# Once that is fixed, the test should succeed.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([COND], [:])
END

cat > Makefile.am << 'END'
if COND
all:
	:
endif
END

$ACLOCAL

AUTOMAKE_fails
grep ':.*overrides.*all' stderr

:
