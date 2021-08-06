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

# Test an example from the manual about the 'amhello' package:
# using VPATH builds to configure the same package with different
# options.

am_create_testdir=empty
required=gcc
. test-init.sh

cp "$am_docdir"/amhello-1.0.tar.gz . \
  || fatal_ "cannot get amhello tarball"

gzip -dc amhello-1.0.tar.gz | tar xf -
cd amhello-1.0

mkdir debug optim
cd debug
../configure CFLAGS='-g -O0'
$MAKE
ls -l . src # For debugging.
cd ../optim
../configure CFLAGS='-O3 -fomit-frame-pointer'
$MAKE
ls -l . src # For debugging.
cd ..

# Check that we have really compiled the objects with two different
# configurations.
for exeext in '' .exe :; do
  test -f optim/src/hello$exeext && break
  test "$exeext" = : && fatal_ "cannot determine extension of executables"
done
cmp optim/src/hello$exeext debug/src/hello$exeext && exit 1

:
