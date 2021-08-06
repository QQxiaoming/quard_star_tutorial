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

# Test remake rules when an m4 file gets renamed and *simultaneously*
# an m4 macro in it gets renamed.  Kudos to Bruno Haible for thinking
# about this situation.  See also related test 'aclocal-deleted-header.sh'.

. test-init.sh

cat >> configure.ac <<'END'
MY_MACRO
AC_OUTPUT
END

cat > Makefile.am <<'END'
ACLOCAL_AMFLAGS = -I m4
.PHONY: test
test:
	test '$(the_answer)' -eq 42
END

mkdir m4

cat > m4/macros.m4 <<'END'
AC_DEFUN([MY_MACRO], [FOO_MACRO])
END

cat > m4/foo.m4 <<'END'
AC_DEFUN([FOO_MACRO], [the_answer=42; AC_SUBST([the_answer])])
END

$ACLOCAL -I m4
$AUTOCONF
$AUTOMAKE

./configure
$MAKE test

# Rename only one file and one macro.

$sleep
sed -e 's/FOO_MACRO/BAR_MACRO/' m4/foo.m4 > m4/bar.m4
rm -f m4/foo.m4
sed -e 's/FOO_MACRO/BAR_MACRO/' m4/macros.m4 > t
mv -f t m4/macros.m4
using_gmake || $MAKE Makefile
$MAKE test
$MAKE distdir
ls -l $distdir $distdir/*
test -f $distdir/m4/bar.m4
test -f $distdir/m4/macros.m4
test ! -e $distdir/m4/foo.m4

# Rename both at once.

$sleep
sed -e 's/BAR_MACRO/QUUX_MACRO/' \
  m4/bar.m4 > m4/quux.m4
sed -e 's/BAR_MACRO/QUUX_MACRO/' -e 's/MY_MACRO/A_MACRO/' \
  m4/macros.m4 > m4/defs.m4
rm -f m4/macros.m4 m4/bar.m4
sed -e 's/BAR_MACRO/QUUX_MACRO/' -e 's/MY_MACRO/A_MACRO/' configure.ac > t
mv -f t configure.ac
using_gmake || $MAKE Makefile
$MAKE test
$MAKE distdir
ls -l $distdir $distdir/*
test -f $distdir/m4/quux.m4
test -f $distdir/m4/defs.m4
test ! -e $distdir/m4/bar.m4
test ! -e $distdir/m4/macros.m4

:
