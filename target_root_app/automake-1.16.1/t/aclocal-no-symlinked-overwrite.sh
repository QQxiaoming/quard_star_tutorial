#! /bin/sh
# Copyright (C) 2006-2018 Free Software Foundation, Inc.
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

# Make sure aclocal does not write into symlinked aclocal.m4.

am_create_testdir=empty
. test-init.sh

echo 'm4_define([FOO], [wrong foo])' > foo.m4
echo 'm4_define([FOO], [right foo])' > acinclude.m4
echo 'AC_INIT FOO' > configure.ac

chmod a-w foo.m4

# Setup.
ln -s foo.m4 aclocal.m4 && test -h aclocal.m4 \
  || skip_ "cannot create symlinks to files"

# Sanity check.
grep 'wrong foo' aclocal.m4 \
  || fatal_ "weird error symlinking aclocal.m4 -> foo.m4"

$ACLOCAL
$AUTOCONF
grep 'right foo' configure
grep 'wrong foo' foo.m4

:
