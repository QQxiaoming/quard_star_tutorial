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

# Test support for DJGPP's .iNN info files.

required=makeinfo
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
@bye
END

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

./configure "--infodir=$(pwd)/_inst"
$MAKE

# Make sure .iNN files are installed.
: > main.i1
: > main.i21
$MAKE install
test -f _inst/main.i1
test -f _inst/main.i21

# They should be uninstalled too.
$MAKE uninstall
test ! -e _inst/main.i1
test ! -e _inst/main.i21

# Make sure rebuild rules erase old .iNN files when they run makeinfo.
$sleep
touch main.texi
test -f main.i1
test -f main.i21
$MAKE
test ! -e main.i1
test ! -e main.i21

# Finally, we also want them erased by maintainer-clean.
: > main.i7
: > main.i39
$MAKE maintainer-clean
test ! -e main.i7
test ! -e main.i39

:
