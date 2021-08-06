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

# Make sure info files survive makeinfo errors.

required=makeinfo
. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am << 'END'
info_TEXINFOS = main.texi sub/main.texi
END

mkdir sub

cat > main.texi << 'END'
\input texinfo
@setfilename main.info
@settitle main
@node Top
Hello walls.
@bye
END

cp main.texi sub/main.texi

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

./configure
$MAKE

# Feign more info files.
: > main.info-1
: > sub/main.info-1

# Break main.texi.
$sleep
cp main.texi main.old
cat > main.texi << 'END'
\input texinfo
@setfilename main.info
@settitle main
@node Top
@unknown_macro{Hello walls.}
@bye
END

# makeinfo will bail out, but we should conserve the old info files.
$MAKE && exit 1
test -f main.info
test -f main.info-1

# Restore main.texi, and break sub/main.texi.
cp main.texi sub/main.texi
mv main.old main.texi
$MAKE && exit 1
test -f main.info
test ! -e main.info-1
test -f sub/main.info
test -f sub/main.info-1

:
