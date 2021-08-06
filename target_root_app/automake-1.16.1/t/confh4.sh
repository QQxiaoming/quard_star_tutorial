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

# Test for PR 34.
# > Description:
#  Automake fails to add -I option to include configuration
#  header indicated like AM_CONFIG_HEADER(magick/config.h)
# > How-To-Repeat:
#  Use AM_CONFIG_HEADER(subdir/config.h) to place configuration
#  header in subdirectory and observe that it is not included.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([include/Makefile])
AC_CONFIG_HEADERS([include/config.h])
AC_PROG_CC
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo
foo_SOURCES = foo.c
END

mkdir include
: > include/Makefile.am
: > include/config.h.in

$ACLOCAL
$AUTOMAKE

grep '^ *DEFAULT_INCLUDES *=.* -I\$(top_builddir)/include' Makefile.in

:
