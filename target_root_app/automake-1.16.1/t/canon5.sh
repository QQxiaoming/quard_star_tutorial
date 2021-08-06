#! /bin/sh
# Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

# Test to make sure that we allow variable names starting in
# non-letters.  Whatever that might mean.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
END

$ACLOCAL

cat > Makefile.am << 'END'
bin_PROGRAMS = 123test
123test_SOURCES = 123.c
END

$AUTOMAKE

cat > Makefile.am << 'END'
bin_PROGRAMS = _foo
_foo_SOURCES = foo.c
END

$AUTOMAKE

cat > Makefile.am << 'END'
bin_PROGRAMS = ,foo
,foo_SOURCES = foo.c
END

AUTOMAKE_fails
grep 'Makefile\.am:2:.* bad .*variable.*,foo_SOURCES' stderr
grep 'Makefile\.am:2:.* use .*_foo_SOURCES' stderr

cat > Makefile.am << 'END'
bin_PROGRAMS = ,foo
_foo_SOURCES = foo.c
END

$AUTOMAKE -Wno-portability

:
