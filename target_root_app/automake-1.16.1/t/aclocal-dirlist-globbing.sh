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

# Check dirlist globbing support.

. test-init.sh

cat > configure.ac <<EOF
AC_INIT([$me], [1.0])
AM_INIT_GUILE_MODULE
AM_FOO_BAR
EOF

mkdir dirlist21-test dirlist22-test

cat >dirlist21-test/dirlist21-check.m4 <<'END'
AC_DEFUN([AM_INIT_GUILE_MODULE],[. $srcdir/../GUILE-VERSION])
END

cat >dirlist22-test/dirlist22-check.m4 <<'END'
AC_DEFUN([AM_FOO_BAR],[
: foo bar baz
])
END

mkdir my-acdir
cat > my-acdir/dirlist <<'END'
dirlist2*-test
END

$ACLOCAL --system-acdir my-acdir
$AUTOCONF

# There should be no m4_include in aclocal.m4, even though m4/dirlist
# contains './dirlist-test' as a relative directory.  Only -I directories
# are subject to file inclusion.
grep m4_include aclocal.m4 && exit 1

grep 'GUILE-VERSION' configure
grep 'foo bar baz' configure

:
