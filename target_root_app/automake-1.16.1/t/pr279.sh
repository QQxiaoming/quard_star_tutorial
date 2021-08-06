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

# Test for PR 279.
# Multiple DEPENDENCIES on conditionals.
# Please keep this in sync with sister test 'pr279-2.sh'.
#
# == Report ==
# When defining xxx_LDADD and xxx_DEPENDENCIES variables where the
# xxx_LDADD one contains values set in conditionals, automake will
# fail with messages like:
#       foo_DEPENDENCIES was already defined in condition TRUE, ...

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_CONDITIONAL([FOOTEST], [false])
AC_OUTPUT
END

cat > Makefile.am << 'END'
if FOOTEST
xtralib =
else
xtralib =
endif

bin_PROGRAMS = foo
foo_LDADD = ${xtralib}
foo_DEPENDENCIES =
END

$ACLOCAL
$AUTOMAKE

:
