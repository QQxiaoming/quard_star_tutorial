#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Check that info files are normally built in $(srcdir),
# not in $(builddir).

required='makeinfo tex texi2dvi'
. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
info_TEXINFOS = main.texi
END

cat > main.texi << 'END'
\input texinfo
@setfilename main.info
@settitle main
@node Top
Hello walls.
@include version.texi
@bye
END

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

mkdir build
cd build
../configure
$MAKE
test -f ../main.info
test ! -e main.info
test -f ../stamp-vti
test ! -e stamp-vti
test -f ../version.texi
test ! -e version.texi

cd ..
rm -rf build make.info* stamp-vti version.texi
./configure
$MAKE
test -f main.info

# Make sure stamp-vti is older that version.texi.
# (A common situation in a real tree.)
# This is needed to test the "subtle" issue described below.
test -f stamp-vti
test -f version.texi
$sleep
touch stamp-vti

$MAKE distclean
test -f stamp-vti
test -f version.texi

mkdir build
cd build
../configure
$MAKE
# main.info should not be rebuilt in the current directory, since
# it's up-to-date in $(srcdir).
# This can be caused by a subtle issue related to VPATH handling
# of version.texi (see also the comment in texi-vers.am): because
# stamp-vti is newer than version.texi, the 'version.texi: stamp-vti'
# rule is always triggered.  Still that's not a reason for 'make'
# to think 'version.texi' has been created...
test ! -e main.info
$MAKE dvi
test -f main.dvi

$MAKE distcheck

:
