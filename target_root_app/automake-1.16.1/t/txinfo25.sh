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

# Check that info files are built in builddir and in srcdir can safely
# co-exist.  This setup is obtained by having two info files, only one
# of which being cleaned.
# (Similar to txinfo16.sh, plus CLEANFILES).
# (See also txinfo23.sh and txinfo24.sh).

required='makeinfo tex texi2dvi'
. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
CLEANFILES = [a-m]*.info
info_TEXINFOS = main.texi other.texi
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

$ACLOCAL
$AUTOCONF

AUTOMAKE_fails --add-missing
grep "Makefile\.am:.*undocumented.* automake hack" stderr
grep "Makefile\.am:.*'info-in-builddir' automake option" stderr

$AUTOMAKE --add-missing -Wno-obsolete

mkdir build
cd build
../configure
$MAKE
test -f main.info
test ! -e ../main.info
test ! -e other.info
test -f ../other.info

cd ..
rm -rf build
./configure
$MAKE
test -f main.info
test -f other.info

# Make sure stamp-vti is older that version.texi.
# (A common situation in a real tree).
# This is needed to test the "subtle" issue described below.
test -f stamp-vti
test -f version.texi
test -f stamp-1
test -f version2.texi
$sleep
touch stamp-vti
touch stamp-1

$MAKE distclean
test -f stamp-vti
test -f stamp-1
test -f version.texi
test -f version2.texi

mkdir build
cd build
../configure
$MAKE
# other.info should not be rebuilt in the current directory, since
# it's up-to-date in $(srcdir).
# This can be caused by a subtle issue related to VPATH handling
# of version.texi (see also the comment in texi-vers.am): because
# stamp-vti is newer than version.texi, the 'version.texi: stamp-vti'
# rule is always triggered.  Still that's not a reason for 'make'
# to think 'version.texi' has been created...
test -f main.info
test ! -e other.info
$MAKE dvi
test -f main.dvi
test -f other.dvi

$MAKE distcheck

:
