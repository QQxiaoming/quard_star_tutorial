#! /bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

# Make sure the MKDIR_P invocation is correct in install-man
# target.  Bug reported by Gordon Irlam <gordoni@cygnus.com>.

. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am << 'EOF'
man_MANS = frob.8
EOF

: > frob.8

$ACLOCAL
$AUTOMAKE

grep 'MKDIR_P' Makefile.in # For debugging.
test $(grep -c '[^(/]MKDIR_P' Makefile.in) -eq 1

:
