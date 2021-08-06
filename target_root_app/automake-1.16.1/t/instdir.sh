#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# If $(foodir) is the empty string, then nothing should be installed there.
# This test only ensures this if $(foo_PRIMARY) is also empty, see
# 'instdir2.sh' and siblings 'instdir-*.sh' for nonempty contents.

. test-init.sh

cat >>configure.ac <<'END'
AC_SUBST([foodir], ['${datadir}'/foo])
AC_OUTPUT
END

cat >Makefile.am <<'END'
bin_SCRIPTS =
nobase_bin_SCRIPTS =
data_DATA =
nobase_data_DATA =
include_HEADERS =
nobase_include_HEADERS =
foo_DATA =
nobase_foo_DATA =
bardir = $(datadir)/bar
bar_DATA =
nobase_bar_DATA =
man1_MANS =
man_MANS =
notrans_man1_MANS =
notrans_man_MANS =
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

cwd=$(pwd) || fatal_ "getting current working directory"
instdir=$cwd/inst
destdir=$cwd/dest
mkdir build
cd build
../configure --prefix="$instdir"
$MAKE

nulldirs='bindir= datadir= includedir= foodir= bardir= man1dir= man2dir='
null_install

:
