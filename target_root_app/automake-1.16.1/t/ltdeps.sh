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

# Test to make sure libtool library dependencies are correct.
# Report from Lars J. Aas.
required=libtool
. test-init.sh

cat >> configure.ac << 'END'
AM_PROG_AR
AC_PROG_LIBTOOL
AC_PROG_CXX
AM_CONDITIONAL(HACKING_COMPACT_BUILD, whatever)
END

cat > Makefile.am << 'END'
RegularSources = \
        SbBSPTree.cpp \
        SbBox2f.cpp \
        SbBox2s.cpp

LinkHackSources = \
        all.cpp

if HACKING_COMPACT_BUILD
BuildSources = $(LinkHackSources)
else
BuildSources = $(RegularSources)
endif

noinst_LTLIBRARIES = libbase.la
libbase_la_SOURCES = $(BuildSources)

EXTRA_libbase_la_SOURCES = \
        $(RegularSources) $(LinkHackSources)
END

: > ltconfig
: > ltmain.sh
: > ar-lib
: > config.guess
: > config.sub

$ACLOCAL
$AUTOMAKE

grep 'OBJECTS =$' Makefile.in && exit 1
exit 0
