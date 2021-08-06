#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Test ACTION-IF-TRUE in AM_PATH_PYTHON.

am_create_testdir=empty
required=python
. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
m4_include([mypy.m4])
AC_OUTPUT
END

# $PYTHON is supposed to be properly set in ACTION-IF-TRUE.
cat > mypy.m4 << 'END'
AM_PATH_PYTHON(, [$PYTHON -V >py-version 2>&1])
END

$ACLOCAL
$AUTOCONF
./configure
grep '^Python [0-9]\.[0-9][0-9]*\.[0-9]' py-version

# The same, but requiring a version.

rm -rf autom4te*.cache

# $PYTHON is supposed to be properly set in ACTION-IF-TRUE.
cat > mypy.m4 << 'END'
AM_PATH_PYTHON([0.0], [$PYTHON -c 'print("%u:%u" % (1-1, 2**0))' > py.out])
END

# The "--force" option here is truly needed to avoid potential timestamp
# races.  See automake bug#12210.
$ACLOCAL --force
$AUTOCONF --force
./configure
test x"$(cat py.out)" = x0:1

:
