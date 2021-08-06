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

# Make sure the user can override TEXINFO_TEX.
# Also make sure TEXINFO_TEX is not distributed.
# Report from Tom Tromey.

required='makeinfo tex texi2dvi'
. test-init.sh

cat > configure.ac << END
AC_INIT([$me], [1.0])
AC_CONFIG_AUX_DIR([aux1])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEXINFO_TEX = ../tex/texinfo.tex
info_TEXINFOS = main.texi
sure_it_exists:
	test -f $(srcdir)/$(TEXINFO_TEX)
sure_it_is_not_distributed: distdir
	test ! -f $(distdir)/tex/texinfo.tex
check-local: sure_it_exists
END

cat > main.texi << 'END'
\input texinfo
@setfilename main.info
@settitle main
@node Top
Hello walls.
@bye
END

mkdir aux1
mkdir tex
cp "$am_pkgvdatadir/texinfo.tex" tex

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

test ! -e texinfo.tex
test ! -e aux1/texinfo.tex
test -f tex/texinfo.tex

./configure

$MAKE distcheck
$MAKE sure_it_is_not_distributed

:
