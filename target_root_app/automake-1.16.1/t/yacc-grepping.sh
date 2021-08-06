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

# Some grep-based checks on yacc support:
#
#  - Make sure intermediate .c file is built from yacc source.
#    Report from Thomas Morgan.
#
#  - Make sure intermediate .h file is not generated nor removed
#    if (AM_)?YFLAGS do not contain '-d'.
#    Requested by Jim Meyering.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_YACC
END

# Run it here once and for all, since we are not going to modify
# configure.ac anymore.
$ACLOCAL

cat > Makefile.am <<'END'
bin_PROGRAMS = zardoz
zardoz_SOURCES = zardoz.y
END

# Don't redefine several times the same variable.
cp Makefile.am Makefile.src

$AUTOMAKE -a
$FGREP 'zardoz.c' Makefile.in
# If zardoz.h IS mentioned, fail.
$FGREP 'zardoz.h' Makefile.in && exit 1

cp Makefile.src Makefile.am
echo 'AM_YFLAGS = -d' >> Makefile.am
$AUTOMAKE
$FGREP 'zardoz.c' Makefile.in
# If zardoz.h is NOT mentioned, fail.
$FGREP 'zardoz.h' Makefile.in

cp Makefile.src Makefile.am
echo 'AM_YFLAGS = ' >> Makefile.am
$AUTOMAKE
$FGREP 'zardoz.c' Makefile.in
# If zardoz.h IS mentioned, fail.
$FGREP 'zardoz.h' Makefile.in && exit 1

cp Makefile.src Makefile.am
echo 'YFLAGS = -d' >> Makefile.am
# YFLAGS is a user variable.
AUTOMAKE_fails
grep 'YFLAGS.* user variable' stderr
grep 'AM_YFLAGS.* instead' stderr
$AUTOMAKE -Wno-gnu
# If zardoz.h is NOT mentioned, fail.
$FGREP 'zardoz.h' Makefile.in

cp Makefile.src Makefile.am
echo 'YFLAGS = ' >> Makefile.am
$AUTOMAKE -Wno-gnu
# If zardoz.h IS mentioned, fail.
$FGREP 'zardoz.h' Makefile.in && exit 1

:
