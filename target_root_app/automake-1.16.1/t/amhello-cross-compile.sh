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
# cross-compiling a package from Linux/Unix to MinGW.

am_create_testdir=empty
required=i586-mingw32msvc-gcc
. test-init.sh

cp "$am_docdir"/amhello-1.0.tar.gz . \
  || fatal_ "cannot get amhello tarball"

host=i586-mingw32msvc
build=$("$am_scriptdir"/config.guess) && test -n "$build" \
  || fatal_ "cannot guess build platform"
case $build in *mingw*) skip_ "build system is MinGW too";; esac

gzip -dc amhello-1.0.tar.gz | tar xf -
cd amhello-1.0

./configure --build "$build" --host "$host" > stdout \
  || { cat stdout ; exit 1; }
cat stdout
grep '^checking for i586-mingw32msvc-strip\.\.\.' stdout
grep '^checking for i586-mingw32msvc-gcc\.\.\.' stdout
grep '^checking for suffix of executables\.\.\. \.exe *$' stdout
grep '^checking for suffix of object files\.\.\. o *$' stdout
grep '^checking whether i586-mingw32msvc-gcc accepts -g\.\.\. yes' stdout

$MAKE

cd src
file hello.exe > whatis
cat whatis
$EGREP 'DOS|Win' whatis
grep 'executable' whatis
grep 'ELF' whatis && exit 1

:
