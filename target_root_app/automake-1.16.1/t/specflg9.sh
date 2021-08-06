#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Another check for per-target flag substitutions.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = zzfoo zzbar
zzfoo_SOURCES = sub/foo.c
zzbar_SOURCES = bar.c
zzbar_CPPFLAGS = -Dfoo
END

$ACLOCAL
$AUTOMAKE --add-missing

$FGREP '$(AM_CFLAGS)' Makefile.in

$FGREP '$(zzfoo_CFLAGS)' Makefile.in && exit 1
$FGREP '$(zzfoo_CPPFLAGS)' Makefile.in && exit 1

$FGREP '$(zzbar_CFLAGS)' Makefile.in && exit 1
$FGREP '$(zzbar_CPPFLAGS)' Makefile.in

:
