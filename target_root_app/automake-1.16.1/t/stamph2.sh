#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Make sure stamp-h* files are created where we expect
. test-init.sh

cat >> configure.ac << END
AC_CONFIG_HEADERS([1.h
                   2.h:config.hin
                   3.h:sdir1/config1.hin])
AC_CONFIG_HEADERS([sdir1/4.h
                   sdir1/5.h:config.hin
                   sdir1/6.h:sdir1/config1.hin
                   sdir1/7.h:sdir2/config2.hin])
AC_OUTPUT
END

: > Makefile.am
mkdir sdir1
mkdir sdir2
: > config.hin
: > 1.h.in
: > sdir1/4.h.in
: > sdir1/config1.hin
: > sdir2/config2.hin

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure

test -f stamp-h1
test -f stamp-h2
test -f stamp-h3
test -f sdir1/stamp-h4
test -f sdir1/stamp-h5
test -f sdir1/stamp-h6
test -f sdir1/stamp-h7

# Make sure './config.status foo' creates the right stamp file.
# Report from Sander Niemeijer.

rm -f stamp-h*
rm -f sdir1/stamp-h*

./config.status sdir1/7.h 2.h sdir1/4.h
test ! -e stamp-h1
test -f stamp-h2
test ! -e stamp-h3
test -f sdir1/stamp-h4
test ! -e sdir1/stamp-h5
test ! -e sdir1/stamp-h6
test -f sdir1/stamp-h7

:
