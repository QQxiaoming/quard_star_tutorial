#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Test for PR automake/229.
# Make sure AC_CYGWIN requires config.{sub,guess}.
# Reported by James Youngman.

. test-init.sh

cat >> configure.ac <<EOF
AC_CYGWIN
EOF

: > Makefile.am

$ACLOCAL
$AUTOMAKE -a

test -f config.sub
test -f config.guess

:
