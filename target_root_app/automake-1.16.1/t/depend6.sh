#! /bin/sh
# Copyright (C) 2008-2018 Free Software Foundation, Inc.
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

# Check for _AM_OUTPUT_DEPENDENCY_COMMANDS grouping bug,
# reported by Markus Duft.

. test-init.sh

cat >>configure.ac << END
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << END
bin_PROGRAMS = foo
foo_SOURCES = foo.c foo.h
END

cat >foo.c << END
#include "foo.h"
END
: >foo.h

$ACLOCAL
$AUTOMAKE
$AUTOCONF
./configure --disable-dependency-tracking 2>stderr || {
  stat=$?; cat stderr >&2; exit $stat;
}
cat stderr >&2
grep shift stderr && exit 1

:
