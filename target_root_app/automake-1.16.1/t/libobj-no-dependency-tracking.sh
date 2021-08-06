#! /bin/sh
# Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

# Ensure that LIBOBJS source files are properly built without dependency
# tracking when using out of tree builds.
#
# This is a non regression test which is following an issue in flex-2.6.4 when
# "malloc.o" was required, see <https://github.com/westes/flex/issues/244>.

. test-init.sh

# The LIBOBJS are in a separate LIBOBJ_DIR directory without anything else in
# it to not trigger the creation of the build directory accidentally.
cat >> configure.ac << 'END'
AC_PROG_CC
AC_CONFIG_LIBOBJ_DIR([foo])
AC_LIBOBJ([foo])
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = helldl
LDADD = $(LIBOBJS)
.PHONY: dummy
dummy: $(LIBOBJS)
END

mkdir foo
cat > foo/foo.c << 'END'
int foo() { return 0; }
END

mkdir build

$ACLOCAL
$AUTOCONF
$AUTOMAKE

cd build
../configure --disable-dependency-tracking
run_make dummy
:
