#! /bin/sh
# Copyright (C) 2000-2018 Free Software Foundation, Inc.
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

# Test for PR 68.
#
# == Report ==
#  If an autoconf substitution variable of the form "*_HEADERS" --
#  for example "MY_HEADERS" -- is used in an "include_" variable
#  in conjunction with EXTRA_HEADERS, $(MY_HEADERS) gets included
#  in the HEADERS variable for *all* generated Makefile.ins in the
#  project.

. test-init.sh

cat >> configure.ac << 'END'
AC_SUBST([MY_HEADERS])
AC_PROG_CC
END

cat > Makefile.am << 'END'
bin_PROGRAMS = zoo
zoo_SOURCES = joe.c
END

: > joe.c
: > qq.h

$ACLOCAL
$AUTOMAKE

grep '[^@]MY_HEADERS' Makefile.in && exit 1
exit 0
