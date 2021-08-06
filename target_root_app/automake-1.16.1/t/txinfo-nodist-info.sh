#! /bin/sh
# Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

# Check that we can force generated '.info' info files not to be
# distributed.

required=makeinfo
. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = info-in-builddir
info_TEXINFOS = foo.texi
CLEANFILES = foo.info

# To make distcheck work without requiring TeX and texi2dvi.
dvi:

# Do not distribute generated '.info' files.
dist-info:
	@:
END

mkdir subdir

cat > foo.texi << 'END'
\input texinfo
@setfilename foo.info
@settitle foo
@node Top
Hello walls.
@include version.texi
@bye
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a -Wno-override

./configure
$MAKE distdir
ls -l . $distdir # For debugging.
test ! -e foo.info
test ! -e $distdir/foo.info
$MAKE
test -f foo.info
$MAKE distdir
ls -l $distdir # For debugging.
test ! -f $distdir/foo.info
$MAKE distcheck

:
