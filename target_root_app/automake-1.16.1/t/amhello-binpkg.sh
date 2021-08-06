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
# using DESTDIR to build simple, no-frills binary packages.

am_create_testdir=empty
. test-init.sh

cp "$am_docdir"/amhello-1.0.tar.gz . \
  || fatal_ "cannot get amhello tarball"

gzip -dc amhello-1.0.tar.gz | tar xf -
cd amhello-1.0

./configure --prefix /usr
$MAKE
$MAKE DESTDIR="$(pwd)/inst" install
cd inst
find . -type f -print > ../files.lst
tar cvf amhello-1.0-i686.tar.gz $(cat ../files.lst) > tar.got 2>&1

EXEEXT=$(sed -n -e 's/^EXEEXT *= *//p' < ../Makefile)

if tar --version </dev/null | grep GNU; then
  LC_ALL=C sort tar.got > t
  mv -f t tar.got
  diff - tar.got <<END
./usr/bin/hello$EXEEXT
./usr/share/doc/amhello/README
END
else
  : Be laxer with other tar implementations, to avoid spurious failures.
  $EGREP '(^| )(\./)?usr/bin/hello'$EXEEXT'( |$)' tar.got
  $EGREP '(^| )(\./)?usr/share/doc/amhello/README( |$)' tar.got
fi

:
