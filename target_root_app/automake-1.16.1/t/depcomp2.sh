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

# Test to make sure that depcomp is not used when it's not installed
# From Pavel Roskin.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([subdir/Makefile])
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
SUBDIRS = subdir
END

rm -f depcomp
mkdir subdir

cat > subdir/Makefile.in << 'END'
foo:
	$(CC) -o foo foo.c
END

: > subdir/foo.c

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF
./configure 2>stderr || { cat stderr >&2; exit 1; }
cat stderr >&2
# Ignore warning messages sometimes seen on Mac OS X; they are
# not automake's fault anyway, but either autoconf's or Mac's.
sed '/rm:.*conftest\.dSYM/d' stderr >stderr2
test -s stderr2 && { cat stderr2; exit 1; }

:
