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

# Test to make sure ylwrap is put in right location.
# Report from Tim Van Holder.

. test-init.sh

mkdir aux1 sub

cat > configure.ac << END
AC_INIT([$me], [1.0])
# 'aux' is not an acceptable file/directory name on Windows systems
AC_CONFIG_AUX_DIR([aux1])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile sub/Makefile])
AC_PROG_CC
AC_PROG_YACC
END

cat > Makefile.am << 'END'
SUBDIRS = sub
bin_PROGRAMS = foo
AM_YFLAGS = -d
foo_SOURCES = foo.y
END

cat > sub/Makefile.am << 'END'
bin_PROGRAMS = bar
AM_YFLAGS = -d
bar_SOURCES = bar.y main.c
END

$ACLOCAL
$AUTOMAKE -a
test -f aux1/ylwrap
test ! -e ylwrap
test ! -e sub/ylwrap
grep -i 'ylwrap' Makefile.in sub/Makefile.in # For debugging.
$FGREP '$(top_srcdir)/aux1/ylwrap' Makefile.in
$FGREP '$(top_srcdir)/aux1/ylwrap' sub/Makefile.in

:
