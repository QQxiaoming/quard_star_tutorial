#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Make sure that '#' after a tab is a failure.
# The Tru64 Unix V5.1 system make will pass these to the
# shell, which in turn can't find '#' as a command.
# Sigh.  Some vendors must be destroyed.

. test-init.sh

cat > Makefile.am << 'END'
install-data-local:
	# Tru64 Unix must die
END

$ACLOCAL
AUTOMAKE_fails
grep 'Makefile.am:2:.*#' stderr
