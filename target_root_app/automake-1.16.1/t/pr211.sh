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

# Test for PR 211.
#
# == Description ==
# Suppose you have a library and an ltlibrary in the same
# directory which use the same source:
#
#   libfoo_la_SOURCES = foo.c
#   libbar_a_SOURCES = foo.c
#
# In this case foo.c will be compiled twice but using different
# compilers.  This is a conflict.  In this case the user should
# rename one of the 'foo.o' files.

required=libtoolize
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_LIBTOOL
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = helldl
lib_LTLIBRARIES = libfoo.la
helldl_SOURCES = foo.c
libfoo_la_SOURCES = foo.c
END

libtoolize
$ACLOCAL
AUTOMAKE_fails -Wnone -a
$FGREP 'foo.$(OBJEXT)' stderr

:
