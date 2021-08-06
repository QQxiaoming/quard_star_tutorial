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

# Test that POSIX variable expansion '$(var:str=rpl)' works when used
# in EXTRA_DIST.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
t1 = foo1 foo2
t2 = bar.x
t3 = baz-y
# Also try an empty match suffix, to ensure that the ':=' in there is
# not confused by the parser with an unportable assignment operator.
EXTRA_DIST = $(t1:=.c) $(t2:.x=.f) $(t3:-y=ar)
bazar bar.f:
	: > $@
.PHONY: test
test: distdir
	ls -l $(distdir)
	test -f $(distdir)/foo1.c
	test -f $(distdir)/foo2.c
	test -f $(distdir)/bar.f
	test -f $(distdir)/bazar
END

: > foo1.c
: > foo2.c

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE test

:
