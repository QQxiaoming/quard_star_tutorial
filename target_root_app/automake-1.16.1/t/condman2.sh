#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Make sure appropriate man install targets generated in all cases.

. test-init.sh

cat > Makefile.am << 'END'
man_MANS = foo.1 foo.2
man5_MANS = foo.5
END

$ACLOCAL
$AUTOMAKE

grep '^install-man1:' Makefile.in
grep '^install-man2:' Makefile.in
grep '^install-man5:' Makefile.in

:
