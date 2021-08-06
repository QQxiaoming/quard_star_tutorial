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

# Make sure we don't override the user's site.exp rule.

. test-init.sh

cat >> configure.ac <<'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = dejagnu

site.exp:
	echo :GREP:ME: > $@
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -Wno-override

grep 'site\.exp' Makefile.in
test $(grep -c '^site\.exp:' Makefile.in) -eq 1

./configure
$MAKE site.exp
grep ':GREP:ME:' site.exp

AUTOMAKE_fails
grep '^Makefile\.am:3:.*site\.exp' stderr

:
