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

# Make sure that Automake choose the shorter route between suffixes
# (related to PR/37)

. test-init.sh

cat >>configure.ac << 'END'
AC_PROG_CC
END

$ACLOCAL

#  x_ -> y -> c -> o
#   \________/
#
# Automake should follow the bottom route: x_ -> c -> o because
# it is shorter.
#
# It should not take the "-> y ->" route.  We use 'y' here so that
# then Automake will complains that YACC is not defined and the test will
# fail when this happens.

cat >Makefile.am << 'END'
bin_PROGRAMS = foo
foo_SOURCES = foo.x_

.x_.y:
	cp $< $@
.x_.c:
	cp $< $@
END

$AUTOMAKE -a

# Idem with the rules the another order.

cat >Makefile.am << 'END'
bin_PROGRAMS = foo
foo_SOURCES = foo.x_

.x_.c:
	cp $< $@
.x_.y:
	cp $< $@
END

$AUTOMAKE -a

:
