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

# Test remake rules when m4 files get moved among different "include
# dirs" (i.e. those passed to aclocal with '-I' option).

. test-init.sh

cat >> configure.ac <<'END'
MY_MACRO
AC_OUTPUT
END

cat > Makefile.am <<'END'
ACLOCAL_AMFLAGS = -I d1 -I d2 -I d3
.PHONY: test
test:
	test '$(the_answer)' -eq 42
END

mkdir d1 d2 d3

cat > d1/macros.m4 <<'END'
AC_DEFUN([MY_MACRO], [FOO])
END

cat > d1/foo.m4 <<'END'
AC_DEFUN([FOO], [the_answer=42; AC_SUBST([the_answer])])
END

$ACLOCAL -I d1
$AUTOCONF
$AUTOMAKE

./configure
$MAKE test

# Move one file.
mv d1/foo.m4 d2/foo.m4
using_gmake || $MAKE Makefile
$MAKE test
$MAKE distdir
ls -l $distdir $distdir/*
test -f $distdir/d2/foo.m4
test ! -e $distdir/d1/foo.m4
test -f $distdir/d1/macros.m4
test ! -e $distdir/d2/macros.m4

# Move both files at once.
mv d1/macros.m4 d3/macros.m4
mv d2/foo.m4 d3/foo.m4
using_gmake || $MAKE Makefile
$MAKE test
$MAKE distdir
ls -l $distdir $distdir/*
test -f $distdir/d3/foo.m4
test -f $distdir/d3/macros.m4
test ! -e $distdir/d1/foo.m4
test ! -e $distdir/d2/foo.m4
test ! -e $distdir/d1/macros.m4
test ! -e $distdir/d2/macros.m4

:
