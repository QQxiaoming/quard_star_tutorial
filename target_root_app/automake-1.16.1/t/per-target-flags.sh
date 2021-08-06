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

# Test executable-specific and library-specific flags, both with
# and without dependency tracking.

. test-init.sh

makefiles='Makefile libMakefile Makefile2 libMakefile2'

cat > configure.ac << END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE([-Wno-extra-portability])
AC_CONFIG_FILES([$makefiles])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_OUTPUT
END

$ACLOCAL

cat > Makefile.am << 'END'
bin_PROGRAMS = foo
foo_SOURCES = foo.c
foo_CFLAGS = -DBAR
END

cat > libMakefile.am << 'END'
lib_LIBRARIES = libfoo.a
libfoo_a_SOURCES = foo.c bar.cc
libfoo_a_CFLAGS = -DBAR
libfoo_a_CXXFLAGS = -DZOT
END

cat - Makefile.am > Makefile2.am << 'END'
AUTOMAKE_OPTIONS = no-dependencies
END

cat - libMakefile.am > libMakefile2.am << 'END'
AUTOMAKE_OPTIONS = no-dependencies
END

makefiles=$(for mkf in $makefiles; do echo $mkf.in; done)

$AUTOMAKE

# Sanity check.
for mkf in $makefiles; do test -f $mkf || exit 99; done

# Regression test for missing space.
$FGREP ')-c' $makefiles && exit 1

# Regression test for botchedly transformed object names.
$FGREP '.o.o' $makefiles && exit 1
$FGREP '.obj.obj' $makefiles && exit 1
$FGREP '.$(OBJEXT).$(OBJEXT)' $makefiles && exit 1

# Watch against non-transformed "foo.$(OBJEXT)", "foo.o" and "foo.obj"
# (and similarly for bar).
$EGREP '[^-](foo|bar)\.[o$]' $makefiles && exit 1

# All our programs and libraries have per-target flags, so all
# the compilers invocations must use an explicit '-c' option.
grep '\$.COMPILE' $makefiles | grep -v ' -c' && exit 1

$FGREP 'foo-foo.$(OBJEXT)' Makefile.in
$FGREP 'foo-foo.$(OBJEXT)' Makefile2.in
$FGREP 'libfoo_a-foo.$(OBJEXT)' libMakefile.in
$FGREP 'libfoo_a-foo.$(OBJEXT)' libMakefile2.in
$FGREP 'libfoo_a-bar.$(OBJEXT)' libMakefile.in
$FGREP 'libfoo_a-bar.$(OBJEXT)' libMakefile2.in

$FGREP '$(foo_CFLAGS)' Makefile.in
$FGREP '$(foo_CFLAGS)' Makefile2.in
$FGREP '$(libfoo_a_CFLAGS)' libMakefile.in
$FGREP '$(libfoo_a_CFLAGS)' libMakefile2.in

:
