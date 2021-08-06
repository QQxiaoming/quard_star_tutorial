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

# Check support for AM_DISTCHECK_CONFIGURE_FLAGS at "make distcheck"
# time, and its interactions with DISTCHECK_CONFIGURE_FLAGS.

. test-init.sh

cat >> configure.ac << 'END'
AC_ARG_ENABLE([success], [], [success=$enableval], [success=no])
AS_IF([test $success = yes && test "$sentence" = 'it works :-)'],
      [:],
      [AC_MSG_ERROR([success='$success', sentence='$sentence'])])
AC_OUTPUT
END

unset sentence

cat > Makefile.am << 'END'
AM_DISTCHECK_CONFIGURE_FLAGS = $(dc_flags1) $(dc_flags2)
dc_flags1 = --enable-success sentence='it works :-)'
dc_flags2 =
END

$ACLOCAL
$AUTOMAKE
$FGREP '$(DISTCHECK_CONFIGURE_FLAGS)' Makefile.in
$FGREP '$(AM_DISTCHECK_CONFIGURE_FLAGS)' Makefile.in
grep 'DISTCHECK_CONFIGURE_FLAGS.*AM_DISTCHECK_CONFIGURE_FLAGS' Makefile.in \
  && exit 1

$AUTOCONF
./configure --enable-success sentence='it works :-)'

# The configure call in "make distcheck" should honour
# $(AM_DISTCHECK_CONFIGURE_FLAGS).

$MAKE distcheck

# $(DISTCHECK_CONFIGURE_FLAGS) takes precedence over
# $(AM_DISTCHECK_CONFIGURE_FLAGS)

cat >> Makefile.am << 'END'
dc_flags2 += --disable-success
END
$AUTOMAKE Makefile
./config.status Makefile

run_make -e FAIL -M distcheck
grep "^configure:.* success='no', sentence='it works :-)'" output

$MAKE distcheck DISTCHECK_CONFIGURE_FLAGS="--enable-success=yes"

:
