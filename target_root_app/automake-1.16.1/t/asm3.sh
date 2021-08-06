#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Test of basic preprocessed assembly functionality with extension '.sx'.
# Keep this in sync with sister tests 'asm.sh' and 'asm2.sh'.

. test-init.sh

mv configure.ac configure.stub

cat > Makefile.am << 'END'
noinst_PROGRAMS = maude
maude_SOURCES = maude.sx
END

echo '*** Try 1 -- should fail because we need CC and CCAS ***'
cat configure.stub - > configure.ac << 'END'
AC_SUBST([CCASFLAGS])
AC_OUTPUT
END

$ACLOCAL
AUTOMAKE_fails
grep '[Aa]ssembler source.*CCAS.* undefined' stderr
grep 'define .*CCAS.* add .*AM_PROG_AS' stderr

# On fast machines the autom4te.cache created during the above run of
# $AUTOMAKE is likely to have the same time stamp as the configure.ac
# created below; thus causing traces for the old configure.ac to be
# used.  We could do '$sleep', but it's faster to erase the
# directory.  (Erase autom4te*.cache, not autom4te.cache, because some
# bogus installations of Autoconf use a versioned cache.)
rm -rf autom4te*.cache

echo '*** Try 2 -- we still need CCAS ***'
cat configure.stub - > configure.ac << 'END'
AC_PROG_CC
AC_SUBST([CCASFLAGS])
AC_OUTPUT
END

$ACLOCAL
AUTOMAKE_fails
grep '[Aa]ssembler source.*CCAS.* undefined' stderr
grep 'define .*CCAS.* add .*AM_PROG_AS' stderr

rm -rf autom4te*.cache

echo '*** Try 3 -- we need CCASFLAGS ***'
cat configure.stub - > configure.ac << 'END'
CCAS='$(CC)'
AC_SUBST([CCAS])
AC_PROG_CC
AC_OUTPUT
END

$ACLOCAL
AUTOMAKE_fails
grep '[Aa]ssembler source.*CCASFLAGS.* undefined' stderr
grep 'define .*CCASFLAGS.* add .*AM_PROG_AS' stderr

rm -rf autom4te*.cache

echo '*** Try 4 -- we need dependency tracking. ***'
cat configure.stub - > configure.ac << 'END'
CCAS='$(CC)'
AC_SUBST([CCAS])
AC_PROG_CC
AC_SUBST([CCASFLAGS])
AC_OUTPUT
END

$ACLOCAL
AUTOMAKE_fails
grep ' add .*AM_PROG_AS' stderr

rm -rf autom4te*.cache

echo '*** Try 5 -- we have everything needed, expect success. ***'
cat configure.stub - > configure.ac << 'END'
CCAS='$(CC)'
AC_SUBST([CCAS])
AC_PROG_CC
_AM_DEPENDENCIES([CCAS])
AC_SUBST([CCASFLAGS])
AC_OUTPUT
END

$ACLOCAL
$AUTOMAKE

rm -rf autom4te*.cache

echo '*** Try 6 -- we have everything needed, expect success. ***'
cat configure.stub - > configure.ac << 'END'
AM_PROG_AS
AC_OUTPUT
END

$ACLOCAL
$AUTOMAKE

:
