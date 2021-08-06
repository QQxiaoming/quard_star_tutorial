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

# Make sure that extra-portability warnings are not enabled by --gnits,
# --gnu and --foreign.

. test-init.sh

# We want complete control over automake options.
AUTOMAKE=$am_original_AUTOMAKE

cat >>configure.ac <<END
AC_PROG_CC
AC_PROG_RANLIB
AC_OUTPUT
END

cat >Makefile.am <<END
EXTRA_LIBRARIES = libfoo.a
libfoo_a_SOURCES = foo.c
END

$ACLOCAL

# Make sure the test is useful.
AUTOMAKE_fails -Wextra-portability \
  || fatal_ "setup doesn't trigger 'extra-portability' warnings"

$AUTOMAKE --foreign

touch INSTALL NEWS README AUTHORS ChangeLog COPYING # Satisfy --gnu.
$AUTOMAKE --gnu

touch THANKS # Satisfy also --gnits.
$AUTOMAKE --gnits

:
