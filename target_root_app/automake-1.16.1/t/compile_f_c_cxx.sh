#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Test to make sure rules to invoke all compilers are selected with
# mixed source objects.
# Matthew D. Langston <langston@SLAC.Stanford.EDU>

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_CXX
AC_PROG_F77
AC_F77_LIBRARY_LDFLAGS
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo
foo_SOURCES  = foo.f bar.c baz.cc
END

: > config.guess
: > config.sub

$ACLOCAL
$AUTOMAKE

# Look for the macros at the beginning of rules.
$FGREP "$tab\$(AM_V_CC)\$(COMPILE)"     Makefile.in
$FGREP "$tab\$(AM_V_CXX)\$(CXXCOMPILE)" Makefile.in
$FGREP "$tab\$(AM_V_F77)\$(F77COMPILE)" Makefile.in

:
