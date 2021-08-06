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

# Test detection of missing Python.
# See also related test t/python-am-path-missing-2.sh (which requires
# an actual python program).

am_create_testdir=empty
# An actual python is *not* required in this test.
. test-init.sh

unset PYTHON

cat > configure.ac <<END
AC_INIT([$me], [1.0])
m4_include([mypy.m4])
AC_OUTPUT
END

echo AM_PATH_PYTHON > mypy.m4

$ACLOCAL
$AUTOCONF

# Simulate no Python.
./configure PYTHON=: 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
grep 'no suitable Python interpreter found' stderr

# Again, but from the environment this time.
env PYTHON=: ./configure 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
grep 'no suitable Python interpreter found' stderr

# Now try using a custom ACTION-IF-NOT-FOUND.

echo 'AM_PATH_PYTHON(,, [echo "$PYTHON" > py])' > mypy.m4
$AUTOCONF --force
./configure PYTHON=:
test x"$(cat py)" = x:

# Now try requiring a version.

rm -rf autom4te*.cache # Will have to re-run aclocal.

# Hopefully the Python team will never release such a version :-)
echo 'AM_PATH_PYTHON([9999.9], [])' > mypy.m4
# The "--force" options (here and below) are truly needed to avoid
# potential timestamp races.  See automake bug#12210.
$ACLOCAL --force
$AUTOCONF --force
./configure >stdout 2>stderr && {
  cat stdout
  cat stderr >&2
  exit 1
}
cat stdout
cat stderr >&2
$EGREP 'checking for a Python interpreter with version >= 9999\.9\.\.\. no(ne)? *$' stdout
grep 'no suitable Python interpreter found' stderr

# Now try requiring a version and using a custom ACTION-IF-NOT-FOUND.

echo 'AM_PATH_PYTHON([9999.9], [], [echo "$PYTHON" > py])' > mypy.m4
$AUTOCONF --force
./configure
test x"$(cat py)" = x:

:
