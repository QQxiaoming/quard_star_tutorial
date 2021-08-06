#! /bin/sh
# Copyright (C) 1997-2018 Free Software Foundation, Inc.
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

# Test to make sure C-but-not-CC error works.

. test-init.sh

cat >> configure.ac << 'END'
dnl AC_PROG_CC and AC_PROG_CXX missing on purpose
AC_LIBOBJ([fsusage])
AC_LIBOBJ([mountlist])
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo
foo_SOURCES = x.C
foo_LDADD = @LIBOBJS@
END

: > fsusage.c
: > mountlist.c

$ACLOCAL
AUTOMAKE_fails
grep ' C source.*CC.* undefined' stderr
grep 'define .*CC.* add .*AC_PROG_CC' stderr
grep ' C++ source.*CXX.* undefined' stderr
grep 'define .*CXX.* add .*AC_PROG_CXX' stderr

:
