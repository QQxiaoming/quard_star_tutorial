#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Expose automake bug#10975: when several "dist-*" targets are invoked
# on the make command line, the first one ends up removing the distdir,
# so that the latter ones fail.

required=bzip2
. test-init.sh

echo AC_OUTPUT >> configure.ac
: > Makefile.am

$ACLOCAL  || fatal_ "aclocal failed"
$AUTOCONF || fatal_ "autoconf failed"
$AUTOMAKE || fatal_ "automake failed"

./configure
$MAKE dist-gzip dist-bzip2

gzip -dc $distdir.tar.gz | tar tf - > one || { cat one; exit 1; }
cat one
bzip2 -dc $distdir.tar.gz | tar tf - > two || { cat two; exit 1; }
cat two

$FGREP "$distdir/Makefile.am" one
diff one two

:
