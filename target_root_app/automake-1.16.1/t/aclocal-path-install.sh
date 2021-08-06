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

# ACLOCAL_PATH and '--install' interactions.

am_create_testdir=empty
. test-init.sh

cat > configure.ac << 'END'
AC_INIT
AM_FOO
END

mkdir acdir pdir ldir

ACLOCAL="$ACLOCAL --system-acdir=$(pwd)/acdir"
ACLOCAL_PATH=./pdir; export ACLOCAL_PATH

cat > acdir/foo.m4 << 'END'
AC_DEFUN([AM_FOO], [:])
END

cat > pdir/bar.m4 << 'END'
AC_DEFUN([AM_BAR], [:])
END

# The '--install' option should never cause anything to be installed
# in a directory specified in ACLOCAL_PATH.
$ACLOCAL --install 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
grep 'impossible without -I .* nor AC_CONFIG_MACRO_DIR' stderr
test ! -e pdir/foo.m4

# The '--install' option should cause a required macro found in a
# directory specified in ACLOCAL_PATH to be installed locally.
echo AM_BAR >> configure.ac
$ACLOCAL --install -I ldir
diff pdir/bar.m4 ldir/bar.m4

:
