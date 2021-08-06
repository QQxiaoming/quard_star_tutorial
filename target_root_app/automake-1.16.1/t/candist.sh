#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Test to make sure things that cannot be dist_'ed are diagnosed.

. test-init.sh

echo AC_OUTPUT >>configure.ac

cat > Makefile.am << 'END'
dist_bin_PROGRAMS = foo
dist_lib_LIBRARIES = libfoo.a
END

$ACLOCAL
AUTOMAKE_fails
test 2 -eq $(grep -c 'dist.*forbidden' stderr)

exit 0
