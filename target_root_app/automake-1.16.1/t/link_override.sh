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

# Test to make sure _LINK variables are detected and used as documented.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo bar baz boo
foo_LINK = $(LINK)
bar_LINK = $(LINK)
bar_LDFLAGS = $(AM_LDFLAGS)
END

$ACLOCAL
$AUTOMAKE -a

# We should use foo_LINK not LINK.
grep '.\$(foo_LINK)' Makefile.in
grep '.\$(LINK).*foo' Makefile.in && exit 1

# We should not override the user definition of bar_LINK.
# IOW, bar_LDFLAGS is useless unless bar_LINK refers to it.
grep '^ *bar_LINK *=.*bar_LDFLAGS' Makefile.in && exit 1
grep '.\$(bar_LINK).*bar' Makefile.in

exit 0
