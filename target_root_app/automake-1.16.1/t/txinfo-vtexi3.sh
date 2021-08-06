#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Check that vers*.texi files are automatically created and distributed
# if @included into a texi source.  Also check that they correctly contain
# the @values definitions they are advertised to.
# See also the related test 'txinfo-vtexi4.sh', which does similar checks,
# but for version.texi only, and requires makeinfo, tex and texi2dvi.

. test-init.sh

# We are going to override package version in AC_INIT, so we need
# to redefine the name of the distdir as well.
distdir=$me-7.45.3a

# This should work without tex, texinfo or makeinfo.
TEX=false TEXI2DVI=false MAKEINFO=false
export TEX TEXI2DVI MAKEINFO

cat > configure.ac << END
AC_INIT([$me], [7.45.3a])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
info_TEXINFOS = foobar.texi quux.texi zardoz.texi
.PHONY: echo-distfiles
echo-distfiles:
	@echo ' ' $(DISTFILES) ' '
END

cat > foobar.texi << 'END'
@setfilename foobar.info
random text
@include version.texi
END

cat > quux.texi << 'END'
@setfilename quux.info
@include version-quux.texi
random text
END

cat > zardoz.texi << 'END'
@setfilename zardoz.info
some randome text
@include vers1a_2b.texi
more random text
END

# Required when using Texinfo.
: > texinfo.tex
cp "$am_scriptdir"/mdate-sh .

$ACLOCAL
$AUTOCONF
$AUTOMAKE

day='([1-9]|1[0-9]|2[0-9]|3[01])'
month='(January|February|March|April|May|June|July|August|September|October|November|December)'
year='20[0-9][0-9]' # Hopefully automake will be obsolete in 80 years ;-)
date="$day $month $year"

do_check ()
{
  # Basename of the vers*.texi file.
  vfile=$1
  # The $(srcdir) of the current build.
  srcdir=$2
  # The vers*.texi file must be created in $(srcdir).
  $MAKE $srcdir/$vfile.texi
  cat $srcdir/$vfile.texi
  # EDITION and VERSION are synonyms, as per documentation.
  grep "^@set EDITION 7\\.45\\.3a$" $srcdir/$vfile.texi
  grep "^@set VERSION 7\\.45\\.3a$" $srcdir/$vfile.texi
  # Check that UPDATED seems right, and that UPDATED and UPDATED-MONTH
  # are consistent.
  $EGREP "^@set UPDATED $date$" $srcdir/$vfile.texi
  vmonth=$(grep '^@set UPDATED ' $srcdir/$vfile.texi | awk '{print $4, $5}')
  grep "^@set UPDATED-MONTH $vmonth$" $srcdir/$vfile.texi
  # Check that the vers*.texi file is distributed according
  # to $(DISTFILES).
  $MAKE echo-distfiles # For debugging.
  $MAKE -s echo-distfiles | grep "[ /]$vfile\\.texi"
}

mkdir build
cd build
../configure

do_check version ..
do_check version-quux ..
do_check vers1a_2b ..

# The various $(srcdir)/*.info are required for the distribution
# and they must be newer than version.texi, so that make won't try
# to rebuild them.
$sleep
touch ../foobar.info
touch ../quux.info
touch ../zardoz.info
# Check that the vers*.texi files are really distributed.
$MAKE distdir
ls -l $distdir
diff ../version.texi $distdir/version.texi
diff ../version-quux.texi $distdir/version-quux.texi
diff ../version.texi $distdir/vers1a_2b.texi

:
