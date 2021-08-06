#!/bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Ensure defining bin_PROGRAMS in terms of EXTRA_PROGRAMS works,
# and that referring to the same program with inconsistent addition
# of $(EXEEXT) works, too.

. test-init.sh

cat >>configure.ac <<'END'
AC_PROG_CC
AC_OUTPUT
END
cat >Makefile.am <<'END'
EXTRA_PROGRAMS = foo bar
bin_PROGRAMS = $(EXTRA_PROGRAMS) baz
noinst_PROGRAMS = baz$(EXEEXT)
END

$ACLOCAL
$AUTOMAKE

:
