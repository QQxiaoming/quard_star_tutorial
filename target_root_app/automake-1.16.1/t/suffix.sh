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

# Make sure proper suffix rules for C compilation are produced, and
# only once.
# See also related test 'suffix2.sh'.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_RANLIB
END

cat > Makefile.am << 'END'
noinst_PROGRAMS = foo
noinst_LIBRARIES = libbar.a
END

for use_arlib in false :; do

  rm -rf autom4te*.cache

  if $use_arlib; then
    am_warns=
    echo AM_PROG_AR >> configure.ac
    : > ar-lib
  else
    am_warns=-Wno-extra-portability
  fi

  $ACLOCAL

  $AUTOMAKE $am_warns -i
  grep '^ *\.c' Makefile.in # For debugging.
  test $(grep -c '^\.c\.o:' Makefile.in) -eq 1
  test $(grep -c '^\.c\.obj:' Makefile.in) -eq 1

  $AUTOMAKE $am_warns
  grep '^ *\.c' Makefile.in # For debugging.
  test $(grep -c '^\.c\.o:' Makefile.in) -eq 1
  test $(grep -c '^\.c\.obj:' Makefile.in) -eq 1

done

:
