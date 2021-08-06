#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Backward-compatibility: AC_OUTPUT with arguments.

. test-init.sh

mkdir sub
: > Makefile.am
: > zardoz
: > foo.in
: > foo1.in
: > foo2.in
: > foo3.in
: > sub/bar.in

ls -l # For debugging.

for args in \
  'foo' \
  'foo:foo1.in' \
  'foo:zardoz' \
  'foo:sub/bar.in' \
  'foo:foo1.in:foo2.in' \
  'foo:zardoz:zardoz' \
  'foo:foo1.in:foo2.in:foo3.in' \
  'foo:foo1.in:foo2.in:sub/bar.in:foo3.in' \
; do
  rm -rf autom4te*.cache aclocal.m4
  unindent >configure.ac <<END
    AC_INIT([$me], [1.0])
    AM_INIT_AUTOMAKE
    AC_CONFIG_FILES([$args])
    AC_OUTPUT
END
  cat configure.ac # For debugging.
  $ACLOCAL
  $AUTOMAKE Makefile
  mv -f Makefile.in Makefile.acf
  rm -rf autom4te*.cache aclocal.m4
  unindent >configure.ac <<END
    AC_INIT([$me], [1.0])
    AM_INIT_AUTOMAKE
    AC_OUTPUT([$args])
END
  cat configure.ac # For debugging.
  $ACLOCAL
  $AUTOMAKE Makefile
  mv -f Makefile.in Makefile.aco
  diff Makefile.acf Makefile.aco
done

:
