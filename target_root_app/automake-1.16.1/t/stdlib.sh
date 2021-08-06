#! /bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

# Test "not a standard library" error.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
END

cat > Makefile.am << 'END'
noinst_LIBRARIES = sub/foo
include $(srcdir)/foo.mk
END

cat > foo.mk << 'END'
## A dummy automake comment.
a = x \
    y
# A dummy make comment.
lib_LIBRARIES = zardoz.a
END

: > ar-lib

$ACLOCAL
AUTOMAKE_fails
badname='not a standard library name'
# We're specifically testing for line-number information.
grep "^Makefile\\.am:1:.*'sub/foo'.*$badname" stderr
grep "^Makefile\\.am:1:.*sub/libfoo\\.a" stderr
grep "^foo\\.mk:5:.*'zardoz\\.a'.*$badname" stderr
grep "^foo\\.mk:5:.*libzardoz\\.a" stderr

:
