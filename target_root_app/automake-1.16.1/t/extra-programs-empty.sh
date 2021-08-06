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

# Test that EXTRA_PROGRAMS doesn't get removed because it is empty.
# This check has been introduced in commit 'Release-1-9-254-g9d0eaef'
# into the former test 'subst2.test'.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([Makefile2 Makefile3])
AC_SUBST([prog])
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am <<'END'
EXTRA_PROGRAMS =
END

cat > Makefile2.am <<'END'
bin_PROGRAMS = a @prog@ b
EXTRA_PROGRAMS =
END

cat > Makefile3.am <<'END'
empty =
EXTRA_PROGRAMS = $(empty)
END

$ACLOCAL
$AUTOMAKE

grep '^EXTRA_PROGRAMS = *$' Makefile.in
grep '^EXTRA_PROGRAMS = *$' Makefile2.in
# Be laxer here, since EXTRA_PROGRAMS might be internally rewritten
# by Automake when it contains references to other variables.
grep '^EXTRA_PROGRAMS =' Makefile3.in

:
