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

# Test of yacc functionality, derived from GNU binutils
# by Tim Van Holder.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_YACC
END

cat > Makefile.am << 'END'
bin_PROGRAMS = maude
maude_SOURCES = sub/maude.y
END

mkdir sub

: > sub/maude.y

$ACLOCAL
# FIXME: stop disabling the warnings in the 'unsupported' category
# FIXME: once the 'subdir-objects' option has been mandatory.
$AUTOMAKE -a -Wno-unsupported

grep '^maude\.c:.*maude\.y' Makefile.in


## Try again with subdir-objects.

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = maude
maude_SOURCES = sub/maude.y
END

$AUTOMAKE -a

# No rule needed, the default .y.c: inference rule is enough
# (but there may be an additional dependency on a dirstamp file).
grep '^sub/maude\.c:.*maude\.y' Makefile.in && exit 1


## Try again with per-exe flags.

cat > Makefile.am << 'END'
bin_PROGRAMS = maude
maude_SOURCES = sub/maude.y
## A particularly trickey case.
maude_YFLAGS = -d
END

# FIXME: stop disabling the warnings in the 'unsupported' category
# FIXME: once the 'subdir-objects' option has been mandatory.
$AUTOMAKE -a -Wno-unsupported

# Rule should use maude_YFLAGS.
grep 'AM_YFLAGS.*maude' Makefile.in && exit 1

# Silly regression.
grep 'maudec' Makefile.in && exit 1

# Make sure the .o file is required.
grep '^am_maude_OBJECTS.*maude' Makefile.in

:
