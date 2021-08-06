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

# Test remake rules when m4 files get renamed.

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
AC_DEFUN([MY_MACRO], [FOO])
END

cat > m4/foo.m4 <<'END'
AC_DEFUN([FOO], [the_answer=42; AC_SUBST([the_answer])])
END

$ACLOCAL -I m4
$AUTOCONF
$AUTOMAKE

./configure
$MAKE test

# Rename one file at the time.

mv m4/foo.m4 m4/bar.m4
using_gmake || $MAKE Makefile
$MAKE test
$MAKE distdir
ls -l $distdir $distdir/*
test -f $distdir/m4/bar.m4
test ! -e $distdir/m4/foo.m4

mv m4/macros.m4 m4/defs.m4
using_gmake || $MAKE Makefile
$MAKE test
$MAKE distdir
ls -l $distdir $distdir/*
test -f $distdir/m4/defs.m4
test ! -e $distdir/m4/macros.m4

# Rename both files at once.

mv m4/bar.m4 m4/quux.m4
mv m4/defs.m4 acinclude.m4
using_gmake || $MAKE Makefile
$MAKE test
$MAKE distdir
ls -l $distdir $distdir/*
test -f $distdir/m4/quux.m4
test -f $distdir/acinclude.m4
test ! -e $distdir/m4/foo.m4
test ! -e $distdir/m4/bar.m4
test ! -e $distdir/m4/macros.m4
test ! -e $distdir/m4/defs.m4

:
