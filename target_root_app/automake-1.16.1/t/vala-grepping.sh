#! /bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

# Basic "grepping" test on vala support.

. test-init.sh

# So that we won't require libtool macros.
cat > acinclude.m4 <<'END'
AC_DEFUN([AC_PROG_LIBTOOL],
         [AC_SUBST([LIBTOOL], [:])])
END

cat >> 'configure.ac' << 'END'
AC_PROG_CC
AC_PROG_CXX
AM_PROG_AR
AC_PROG_LIBTOOL
AM_PROG_VALAC
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

cat > Makefile.am <<'END'
SUBDIRS = sub

bin_PROGRAMS = zardoz
zardoz_SOURCES = zardoz.vala
zardoz_VALAFLAGS = --debug

lib_LTLIBRARIES = libzardoz.la
libzardoz_la_SOURCES = zardoz-foo.vala zardoz-bar.vala
END

mkdir sub

cat > sub/Makefile.am <<'END'
bin_PROGRAMS = foo
foo_SOURCES = bar.vala baz.vala mu.c zap.cxx
END

: > ltmain.sh
: > config.sub
: > config.guess

$ACLOCAL
$AUTOMAKE -a

grep '\$(VALAC).* \$(AM_VALAFLAGS) \$(VALAFLAGS) ' Makefile.in
grep '\$(VALAC).* \$(zardoz_VALAFLAGS) \$(VALAFLAGS) ' Makefile.in
$FGREP 'am_zardoz_OBJECTS' Makefile.in
$FGREP 'am_libzardoz_la_OBJECTS' Makefile.in
$FGREP 'zardoz_vala.stamp:' Makefile.in
$FGREP 'libzardoz_la_vala.stamp:' Makefile.in
test $($FGREP -c '.stamp:' Makefile.in) -eq 2
$FGREP 'zardoz.c' Makefile.in
$FGREP 'zardoz-foo.c' Makefile.in
$FGREP 'zardoz-bar.c' Makefile.in

grep '\$(VALAC).* \$(AM_VALAFLAGS) \$(VALAFLAGS) ' sub/Makefile.in
$FGREP 'foo_VALAFLAGS' sub/Makefile.in && exit 1
$FGREP 'am_foo_OBJECTS' sub/Makefile.in
$FGREP 'bar.c' sub/Makefile.in
$FGREP 'baz.c' sub/Makefile.in
$FGREP 'foo_vala.stamp:' sub/Makefile.in
test $($FGREP -c '.stamp:' sub/Makefile.in) -eq 1

# Check against regression for weird bug due to unescaped '@'
# characters used in a "..." perl string when writing the vala
# rules from automake.in.
grep '\$[0-9][0-9]*t' Makefile.in sub/Makefile.in && exit 1

:
