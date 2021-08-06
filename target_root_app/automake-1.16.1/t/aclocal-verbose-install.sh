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

# Check verbose messages by 'aclocal --install'.

am_create_testdir=empty
. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
MY_MACRO_BAR
MY_MACRO_QUUX
END

mkdir sys-acdir
cat > sys-acdir/bar.m4 <<END
AC_DEFUN([MY_MACRO_BAR], [:])
END
cat > sys-acdir/quux.m4 <<END
AC_DEFUN([MY_MACRO_QUUX], [:])
END

mkdir foodir
: > foodir/bar.m4

$ACLOCAL --system-acdir=sys-acdir --install --verbose -I foodir 2>stderr \
 || { cat stderr >&2; exit 1; }
cat stderr >&2
grep ' installing .*sys-acdir/bar\.m4.* to .*foodir/bar\.m4' stderr
grep ' installing .*sys-acdir/quux\.m4.* to .*foodir/quux\.m4' stderr
grep ' overwriting .*foodir/bar\.m4.* with .*sys-acdir/bar\.m4' stderr
grep ' installing .*foodir/quux\.m4.* from .*sys-acdir/quux\.m4' stderr

# Sanity checks.
ls -l foodir
grep MY_MACRO_BAR foodir/bar.m4
grep MY_MACRO_QUUX foodir/quux.m4

:
