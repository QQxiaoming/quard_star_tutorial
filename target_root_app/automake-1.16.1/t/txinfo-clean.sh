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

# DVIS, PDFS, PSS, HTMLS should not be cleaned upon 'mostlyclean'.
# Similar to txinfo25.sh.

required='makeinfo tex texi2dvi dvips'
. test-init.sh

mkdir sub

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
info_TEXINFOS = main.texi other.texi sub/another.texi
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

cat > other.texi << 'END'
\input texinfo
@setfilename other.info
@settitle other
@node Top
Hello walls.
@include version2.texi
@bye
END

cat > sub/another.texi << 'END'
\input texinfo
@setfilename another.info
@settitle another
@node Top
Hello walls.
@include version3.texi
@bye
END

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

# In-tree build.
./configure
$MAKE dvi ps pdf html

test -f main.dvi
test -f main.ps
test -f main.html || test -d main.html
test -f main.pdf
test -f other.pdf
test -f sub/another.pdf

test -f other.dvi
test -f other.html || test -d other.html
test -f other.ps
test -f sub/another.dvi
test -f sub/another.html || test -d sub/another.html
test -f sub/another.ps

$MAKE mostlyclean

ls *.aux && exit 1
ls sub/*.aux && exit 1

test -f main.dvi
test -f main.ps
test -f main.html || test -d main.html
test -f main.pdf
test -f other.pdf
test -f sub/another.pdf

test -f other.dvi
test -f other.html || test -d other.html
test -f other.ps
test -f sub/another.dvi
test -f sub/another.html || test -d sub/another.html
test -f sub/another.ps

$MAKE clean

test ! -e main.dvi
test ! -e main.ps
test ! -e main.html
test ! -e main.pdf
test ! -e other.pdf
test ! -e sub/another.pdf
test ! -e sub/yetanother.pdf

test ! -e other.dvi
test ! -e other.html
test ! -e other.ps
test ! -e sub/another.dvi
test ! -e sub/another.html
test ! -e sub/another.ps

./configure
$MAKE

$MAKE distcheck

:
