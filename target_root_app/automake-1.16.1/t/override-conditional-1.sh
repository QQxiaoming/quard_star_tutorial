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

# Test for conditionally-defined overrides.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([COND], [test x"$cond" = x"yes"])
AC_OUTPUT
END

cat > Makefile.am << 'END'
foobar:
	: > $@
if COND
ps: foobar
	:
endif
END

$ACLOCAL
$AUTOMAKE -Wno-override

# "ps:" should be output in two conditions
test $(grep -c '[^-]ps:' Makefile.in) -eq 2
grep '@COND_TRUE@ps: *foobar' Makefile.in
grep '@COND_FALSE@ps: *ps-am' Makefile.in

$AUTOCONF

./configure cond=no
$MAKE ps
test ! -e foobar

./configure cond=yes
$MAKE ps
test -f foobar

:
