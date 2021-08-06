#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Check dirlist support.

. test-init.sh

cat > configure.ac <<EOF
AC_INIT([$me], [1.0])
AM_INIT_GUILE_MODULE
EOF

mkdir acdir dirlist-test
echo ./dirlist-test > acdir/dirlist

cat >dirlist-test/dirlist-check.m4 <<'END'
AC_DEFUN([AM_INIT_GUILE_MODULE],[
. $srcdir/../GUILE-VERSION
AM_INIT_AUTOMAKE(foreign)
AC_CONFIG_AUX_DIR(..)
module=[$1]
AC_SUBST(module)])
END

cat >dirlist-test/init.m4 <<EOF
AC_DEFUN([AM_INIT_AUTOMAKE], [I should not be included])
EOF

$ACLOCAL --system-acdir acdir
$AUTOCONF

# There should be no m4_include in aclocal.m4, even though m4/dirlist
# contains './dirlist-test' as a relative directory.  Only -I directories
# are subject to file inclusion.
grep m4_include aclocal.m4 && exit 1

grep 'GUILE-VERSION' configure

# This bug can occur only when we do a VPATH build of Automake
# but it's OK because VPATH builds are done by 'make distcheck'.
grep 'I should not be included' configure && exit 1

:
