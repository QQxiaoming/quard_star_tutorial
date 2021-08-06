#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Test to make sure config.h rule made even if it is in a subdir.  The
# idea is that if config.h is in a subdir, and there is no Makefile in
# that subdir, then we want to build config.h as the top level.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_HEADERS([subdir/config.h])
AC_OUTPUT
END

: > Makefile.am
mkdir subdir
: > subdir/config.h.in

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure
$MAKE

$sleep
echo '#define gRePmE' > subdir/config.h.in
$MAKE subdir/config.h
$FGREP gRePmE subdir/config.h

$MAKE distcheck

:
