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

# Check that info files are built in builddir when needed.
# Test with subdir Texinfo.
# (Similar to txinfo13.sh, plus DISTCLEANFILES).
# (See also txinfo24.sh and txinfo25.sh).

required='makeinfo tex texi2dvi'
. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
DISTCLEANFILES = subdir/*.info*
info_TEXINFOS = subdir/main.texi
subdir_main_TEXINFOS = subdir/inc.texi

installcheck-local:
	test -f "$(infodir)/main.info"
END

mkdir subdir

cat > subdir/main.texi << 'END'
\input texinfo
@setfilename main.info
@settitle main
@node Top
Hello walls.
@include version.texi
@include inc.texi
@bye
END

cat > subdir/inc.texi << 'END'
I'm included.
END

$ACLOCAL
$AUTOCONF

AUTOMAKE_run --add-missing -Wno-error
grep "Makefile\.am:.*undocumented.* automake hack" stderr
grep "Makefile\.am:.*'info-in-builddir' automake option" stderr

mkdir build
cd build
../configure
$MAKE distcheck
test -f subdir/main.info
test ! -e ../subdir/main.info

:
