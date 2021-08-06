#! /bin/sh
# Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

# Check that Automake doesn't generated rules or definitions related
# to compilation of C sources for a project that doesn't use nor need
# a C Compiler.  Inspired by the issues reported in automake bug#14560.

am_create_testdir=empty
. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([
    Makefile
    sub/Makefile
    sub2/Makefile
])
AC_PROG_CXX
AC_PROG_F77
AC_OUTPUT
END

cat > Makefile.am <<END
SUBDIRS = sub sub2
bin_PROGRAMS = bar
bar_SOURCES = bar.cc
END

mkdir sub sub2

cat > sub/Makefile.am <<END
bin_PROGRAMS = foo
foo_SOURCES = foo.f
END

cat > sub2/Makefile.am <<END
bin_PROGRAMS = baz
baz_SOURCES = baz.cxx
END


$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

test   -f install-sh
test ! -e compile

# Sanity checks.
$FGREP '$(CXX)' Makefile.in
$FGREP '$(CXX)' sub2/Makefile.in
$FGREP '$(F77)' Makefile.in sub2/Makefile.in && exit 1
$FGREP '$(F77)' sub/Makefile.in
$FGREP '$(CXX)' sub/Makefile.in && exit 1

$EGREP '(^COMPILE|$\(CC\)|AM_V_CC)' \
  Makefile.in sub/Makefile.in sub2/Makefile.in && exit 1

:
