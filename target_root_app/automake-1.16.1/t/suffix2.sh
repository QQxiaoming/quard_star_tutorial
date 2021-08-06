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

# Make sure proper suffix rules for C compilation are produced,
# and only once, even for libtool libraries.
# See also related test 'suffix.sh'.
required=libtoolize

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_LIBTOOL
END

cat > Makefile.am << 'END'
lib_LTLIBRARIES = libltdl.la
libltdl_la_SOURCES = ltdl.c ltdl.h
END

# Create all the files created libtoolize so we don't run libtoolize.
# We don't want the user to need libtool to run this test.
: > ltconfig
: > ltmain.sh
: > config.guess
: > config.sub

$ACLOCAL

$AUTOMAKE -a
grep '^ *\.c' Makefile.in # For debugging.
test $(grep -c '^\.c\.o:' Makefile.in) -eq 1
test $(grep -c '^\.c\.obj:' Makefile.in) -eq 1

$AUTOMAKE -i
grep '^ *\.c' Makefile.in # For debugging.
test $(grep -c '^\.c\.o:' Makefile.in) -eq 1
test $(grep -c '^\.c\.obj:' Makefile.in) -eq 1

:
