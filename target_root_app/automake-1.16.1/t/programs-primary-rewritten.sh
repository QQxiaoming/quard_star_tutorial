#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Make sure xxx_PROGRAMS gets properly rewritten where needed.  These
# checks have been introduced in commit 'Release-1-9-254-g9d0eaef' into
# the former test 'subst2.test'.

. test-init.sh

cat >> configure.ac << 'END'
AC_SUBST([FOO], [c])
AC_OUTPUT
END

cat >Makefile.am <<'END'
AUTOMAKE_OPTIONS = no-dependencies
CC = false
EXEEXT = .bin

check_PROGRAMS = a
bin_PROGRAMS = b @FOO@
noinst_PROGRAMS = $(bar) $(baz:=de)
EXTRA_PROGRAMS =

bar = zardoz
baz = mau

.PHONY: test
test:
	is $(check_PROGRAMS) == a.bin
	is $(bin_PROGRAMS) == b.bin c
	is $(noinst_PROGRAMS) == zardoz.bin maude.bin
END

$ACLOCAL
$AUTOMAKE

grep PROGRAMS Makefile.in # For debugging.

# Check that no useless indirections are used.
grep '^check_PROGRAMS = a$(EXEEXT)$' Makefile.in
grep '^bin_PROGRAMS = b$(EXEEXT) @FOO@$' Makefile.in

$AUTOCONF
./configure
$MAKE test

:
