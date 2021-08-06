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

# Test to make sure '.' in an exe name doesn't fool us.
# Report from Robert Collins.
# Also make sure we rewrite conditionals variables.
# Also check for PR/352.
# Also make sure the old definitions of bin_PROGRAMS and friend
# aren't left around.  Report from Jim Meyering.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_CONDITIONAL([WANT_MT], [test -z "$revert"])
AM_CONDITIONAL([WANT_RMT], [test -z "$revert"])
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = maude
sbin_PROGRAMS = maude.static
## We don't define this one for now.  Probably it is an error.
## noinst_PROGRAMS = maude2.exe
check_PROGRAMS = maude3$(EXEEXT)

if WANT_MT
  bin_PROGRAMS += mt
endif
if WANT_RMT
  libexec_PROGRAMS = rmt
endif

test-default:
	is $(bin_PROGRAMS)      ==  maude$(EXEEXT) mt$(EXEEXT)
	is $(sbin_PROGRAMS)     ==  maude.static$(EXEEXT)
	is $(check_PROGRAMS)    ==  maude3$(EXEEXT)
	is $(libexec_PROGRAMS)  ==  rmt$(EXEEXT)

test-revert:
	is $(bin_PROGRAMS)      ==  maude$(EXEEXT)
	is $(sbin_PROGRAMS)     ==  maude.static$(EXEEXT)
	is $(check_PROGRAMS)    ==  maude3$(EXEEXT)
	is $(libexec_PROGRAMS)  ==
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

grep '^maude$(EXEEXT):' Makefile.in
grep '^maude\.static$(EXEEXT):' Makefile.in
grep '^maude3$(EXEEXT):' Makefile.in
grep '^mt$(EXEEXT):' Makefile.in
grep '^rmt$(EXEEXT):' Makefile.in
test $(grep -c '^bin_PROGRAMS =' Makefile.in) -eq 1

# Make sure $(EXEEXT) gets stripped before canonicalization.
grep 'maude3__EXEEXT__OBJECTS' Makefile.in && exit 1

./configure
run_make test-default
run_make test-default EXEEXT=.foo

./configure revert=yes
run_make test-revert
run_make test-revert EXEEXT=.foo

:
