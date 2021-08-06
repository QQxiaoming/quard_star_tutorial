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

# Check for false positives in automake recognition of '-d' in YFLAGS.

. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AC_PROG_YACC
END

$ACLOCAL

cat > Makefile.am <<'END'
bin_PROGRAMS = zardoz foobar
zardoz_SOURCES = zardoz.y
foobar_SOURCES = foobar.y
# All the "almost -d" substrings ("- d", "-dd", etc.) are meant.
AM_YFLAGS = -xd --d - d --output=d
foobar_YFLAGS = - d $(foovar)-d -dd
END

$AUTOMAKE -a
$EGREP '(foobar|zardoz)\.h.*:' Makefile.in && exit 1
$EGREP '(foobar|zardoz)\.h' Makefile.in | $FGREP -v '$(YLWRAP) ' && exit 1

:
