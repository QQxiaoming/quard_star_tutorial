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

# Test to make sure depcomp is installed and found properly
# when required for multiple directories.

. test-init.sh

cat > configure.ac << END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([subdir/Makefile subdir2/Makefile])
AC_PROG_CC
AC_OUTPUT
END

rm -f depcomp
mkdir subdir
mkdir subdir2

cat > subdir/Makefile.am << 'END'
noinst_PROGRAMS = foo
foo_SOURCES = foo.c
END

cp subdir/Makefile.am subdir2/Makefile.am

$ACLOCAL
$AUTOMAKE --add-missing

# There used to be a bug where this was created in the first subdir with C
# sources in it instead of in $top_srcdir or $ac_auxdir.
test -f depcomp

:
