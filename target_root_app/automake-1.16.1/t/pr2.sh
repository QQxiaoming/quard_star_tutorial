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

# Test for bug reported in PR 2.
# "make dist" fails for AC_OUTPUT(README.foo:templ/README.foo.in).
#
# == Report ==
# When my AC_OUTPUT macro in configure.ac contains something
# like the following (among the Makefiles)
#       README.foo:templ/README.foo.in
# the generated dist target in the Makefile.in fails,
# because DIST_COMMON will contain templ/README.foo.in
# and the dist target will try to copy templ/README.foo.in
# to the distribution directory before it creates a templ
# subdirectory in the distribution directory.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT([README.foo:templ/README.foo.in])
END

: > Makefile.am

mkdir templ
: > templ/README.foo.in

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure
$MAKE distdir
test -f $distdir/templ/README.foo.in

:
