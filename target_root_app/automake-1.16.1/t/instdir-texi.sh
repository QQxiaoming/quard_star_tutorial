#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# If $(infodir) is the empty string, then nothing should be installed there.
# Likewise for the other install-* targets used for texinfo files.

required='makeinfo tex texi2dvi'
. test-init.sh

dvips --help \
  || skip_ "dvips is missing"
pdfetex --version || pdftex --version \
  || skip_ "pdeftex and pdftex are both missing"

cat >>configure.ac <<'END'
AC_OUTPUT
END

cat >Makefile.am <<'END'
info_TEXINFOS = foo.texi
END

cat >foo.texi << 'END'
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
$AUTOMAKE --add-missing

cwd=$(pwd) || fatal_ "getting current working directory"
instdir=$cwd/inst
destdir=$cwd/dest
mkdir build
cd build
../configure --prefix="$instdir"
$MAKE all dvi ps pdf html
ls -l

nulldirs='infodir= htmldir= dvidir= psdir= pdfdir='
null_install --texi

:
