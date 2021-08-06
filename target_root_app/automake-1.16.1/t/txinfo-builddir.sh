#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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
# This test that this can be done through the so far undocumented
# option 'info-in-builddir', as requested by at least GCC, GDB,
# GNU binutils and the GNU bfd library.  See automake bug#11034.

required='makeinfo tex texi2dvi'
. test-init.sh

if useless_vpath_rebuild; then
  skip_ "$MAKE has brittle VPATH support"
fi

echo AC_OUTPUT >> configure.ac

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = info-in-builddir
info_TEXINFOS = foo.texi subdir/bar.texi mu.texi
subdir_bar_TEXINFOS = subdir/inc.texi
CLEANFILES = mu.info

# mu.info should not be rebuilt in the current directory, since
# it's up-to-date in $(srcdir).
# This can be caused by a subtle issue related to VPATH handling
# of version.texi (see also the comment in texi-vers.am): because
# stamp-vti is newer than version.texi, the 'version.texi: stamp-vti'
# rule is always triggered.  Still that's not a reason for 'make'
# to think 'version.texi' has been created...
check-local:
	test ! -e mu.info
	test -f $(srcdir)/mu.info
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

cat > mu.texi << 'END'
\input texinfo
@setfilename mu.info
@settitle mu
@node Top
Mu mu mu.
@bye
END

cat > subdir/bar.texi << 'END'
\input texinfo
@setfilename bar.info
@settitle bar
@node Top
Hello walls.
@include inc.texi
@bye
END

echo "I'm included." > subdir/inc.texi

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

mkdir build
cd build
../configure
$MAKE info
test -f foo.info
test -f subdir/bar.info
test -f mu.info
test -f ../stamp-vti
test -f ../version.texi
test ! -e ../foo.info
test ! -e ../subdir/bar.info
test ! -e ../mu.info
$MAKE clean
test -f foo.info
test -f subdir/bar.info
test ! -e mu.info
test -f ../stamp-vti
test -f ../version.texi

# Make sure stamp-vti is older that version.texi.
# (A common situation in a real tree).
$sleep
touch ../stamp-vti

$MAKE distcheck
# Being distributed, this file should have been rebuilt.
test -f mu.info

$MAKE distclean
test -f ../stamp-vti
test -f ../version.texi
test -f foo.info
test -f subdir/bar.info
test ! -e mu.info

../configure
$MAKE maintainer-clean
test ! -e ../stamp-vti
test ! -e ../version.texi
test ! -e stamp-vti
test ! -e version.texi
test ! -e foo.info
test ! -e subdir/bar.info
test ! -e mu.info

:
