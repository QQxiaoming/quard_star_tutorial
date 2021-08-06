#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Check support for DISTCHECK_CONFIGURE_FLAGS at "make distcheck" time.

. test-init.sh

cat >> configure.ac << 'END'
AC_ARG_ENABLE([success], [], [success=$enableval], [success=no])
AS_IF([test $success = yes && test "$sentence" = 'it works :-)'],
      [:],
      [AC_MSG_ERROR([success='$success', sentence='$sentence'])])
AC_OUTPUT
END

unset sentence

: > Makefile.am

$ACLOCAL
$AUTOMAKE
$FGREP '$(DISTCHECK_CONFIGURE_FLAGS)' Makefile.in

$AUTOCONF
./configure --enable-success sentence='it works :-)'

# It should be ok to define DISTCHECK_CONFIGURE_FLAGS either on the
# make command line or in the environment.

env DISTCHECK_CONFIGURE_FLAGS='--enable-success sentence=it\ works\ :-\)' \
  $MAKE distcheck

$MAKE distcheck \
  DISTCHECK_CONFIGURE_FLAGS="--enable-success=yes sentence='it works :-)'"

# Sanity check.
run_make -M -e FAIL distcheck
grep "^configure:.* success='no', sentence=''" output

:
