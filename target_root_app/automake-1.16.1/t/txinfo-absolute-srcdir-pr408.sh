#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Make sure Texinfo installation works when absolute --srcdir is used.
# PR/408

required='makeinfo'
. test-init.sh

echo AC_OUTPUT >> configure.ac

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

./configure
$MAKE
$MAKE distclean

abscwd=$(pwd) || fatal_ "getting current working directory"

mkdir build
cd build
../configure --srcdir="$abscwd" \
             --prefix="$abscwd/build/_inst" \
	     --infodir="$abscwd/build/_inst/info"

$MAKE install

test -f ../main.info
test ! -e ./main.info
test -f _inst/info/main.info

$MAKE uninstall
test ! -e _inst/info/main.info
test -f ../main.info

# Multiple uninstall should not fail.
$MAKE uninstall

:
