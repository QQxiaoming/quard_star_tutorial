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

# Test support for building HTML documentation, and the many
# install-DOC flavors, in VPATH builds.
# Keep in sync with sister test 'txinfo-many-output-formats.sh'.
# FIXME: in the long term, the best thing to do is probably to convert
# FIXME: this test and that sister test to TAP, and merge them.

required='makeinfo tex texi2dvi'
. test-init.sh

cat >>configure.ac <<\EOF
AC_CONFIG_FILES([rec/Makefile])

# At the time of writing, Autoconf does not supplies any of these
# definitions (and those below are purposely not those of the standard).
AC_SUBST([dvidir],  ['${datadir}/${PACKAGE}/dvi'])
AC_SUBST([htmldir], ['${datadir}/${PACKAGE}/html'])
AC_SUBST([pdfdir],  ['${datadir}/${PACKAGE}/pdf'])
AC_SUBST([psdir],   ['${datadir}/${PACKAGE}/ps'])

AC_OUTPUT
EOF

cat > Makefile.am << 'END'
SUBDIRS = rec
info_TEXINFOS = main.texi sub/main2.texi
END

cat > main.texi << 'END'
\input texinfo
@setfilename main.info
@settitle main
@node Top
Hello walls.
@bye
END

mkdir sub
cat > sub/main2.texi << 'END'
\input texinfo
@setfilename main2.info
@settitle main2
@node Top
Hello walls.
@bye
END

mkdir rec
cat > rec/main3.texi << 'END'
\input texinfo
@setfilename main3.info
@settitle main3
@node Top
Hello walls.
@bye
END

cat > rec/Makefile.am << 'END'
info_TEXINFOS = main3.texi

install-pdf-local:
	@$(MKDIR_P) "$(pdfdir)"
	: > "$(pdfdir)/hello"
uninstall-local:
	rm -f "$(pdfdir)/hello"

check-local: ps pdf dvi html # For "make distcheck".
END

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

# To simplify syncing with sister test 'txinfo-many-output-formats.sh'
srcdir=..

if test $srcdir = ..; then
  mkdir build
  cd build
fi

$srcdir/configure --prefix="$(pwd)"

$MAKE

$sleep
# Test production of split-per-node HTML.
$MAKE html
test -d main.html
test -d sub/main2.html
test -d rec/main3.html

# Rebuilding main.html should cause its timestamp to be updated.
is_newest main.html $srcdir/main.texi
$sleep
touch $srcdir/main.texi
$MAKE html
is_newest main.html $srcdir/main.texi

$MAKE clean
test ! -e main.html
test ! -e sub/main2.html
test ! -e rec/main3.html

# Test production of a single HTML file.
run_make MAKEINFOFLAGS=--no-split html
test -f main.html
test -f sub/main2.html
test -f rec/main3.html
$MAKE clean
test ! -e main.html
test ! -e sub/main2.html
test ! -e rec/main3.html

# Make sure AM_MAKEINFOHTMLFLAGS is supported, and override AM_MAKEINFO.

cp $srcdir/Makefile.am $srcdir/Makefile.sav
cat >> $srcdir/Makefile.am <<'EOF'
AM_MAKEINFOHTMLFLAGS = --no-headers --no-split
AM_MAKEINFOFLAGS = --unsupported-option
EOF
(cd $srcdir && $AUTOMAKE)
./config.status Makefile

$MAKE html
test -f main.html
test -f sub/main2.html
test -d rec/main3.html
$MAKE clean
test ! -e main.html
test ! -e sub/main2.html
test ! -e rec/main3.html

$MAKE install-html
test -f share/$me/html/main.html
test -f share/$me/html/main2.html
test -d share/$me/html/main3.html
$MAKE uninstall
test ! -e share/$me/html/main.html
test ! -e share/$me/html/main2.html
test ! -e share/$me/html/main3.html

$MAKE dvi
test -f main.dvi
test -f sub/main2.dvi
test -f rec/main3.dvi
$MAKE clean
test ! -e main.dvi
test ! -e sub/main2.dvi
test ! -e rec/main3.dvi

$MAKE install-dvi
test -f share/$me/dvi/main.dvi
test -f share/$me/dvi/main2.dvi
test -f share/$me/dvi/main3.dvi
$MAKE uninstall
test ! -e share/$me/dvi/main.dvi
test ! -e share/$me/dvi/main2.dvi
test ! -e share/$me/dvi/main3.dvi

dvips --help || skip_ "dvips is missing"

$MAKE install-ps
test -f share/$me/ps/main.ps
test -f share/$me/ps/main2.ps
test -f share/$me/ps/main3.ps
$MAKE uninstall
test ! -e share/$me/ps/main.ps
test ! -e share/$me/ps/main2.ps
test ! -e share/$me/ps/main3.ps

pdfetex --help || pdftex --help \
  || skip_ "pdftex and pdfetex are both missing"

$MAKE install-pdf
test -f share/$me/pdf/main.pdf
test -f share/$me/pdf/main2.pdf
test -f share/$me/pdf/main3.pdf
test -f share/$me/pdf/hello
$MAKE uninstall
test ! -e share/$me/pdf/main.pdf
test ! -e share/$me/pdf/main2.pdf
test ! -e share/$me/pdf/main3.pdf
test ! -e share/$me/pdf/hello

# Restore the makefile without a broken AM_MAKEINFOFLAGS definition.
cp -f $srcdir/Makefile.sav $srcdir/Makefile.am
using_gmake || $MAKE Makefile
$MAKE distcheck

:
