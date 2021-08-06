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

# Make sure aclocal define macros in the same order as -I's.
# This is the similar to aclocal-I-order-1.sh, with the macro calls
# reversed (it did make a difference).
#
# Also check for --install.

# TODO: write a sister test that doesn't use a 'dirlist' file, but
# TODO: puts third-party macros directly into 'acdir'.

am_create_testdir=empty
. test-init.sh

cat > configure.ac << 'END'
AC_INIT
MACRO2
MACRO1
MACRO3
END

ACLOCAL="$ACLOCAL --system-acdir acdir"

mkdir m4_1 m4_2 acdir acdir2
echo ./acdir2 > acdir/dirlist

cat >m4_1/somedefs.m4 <<EOF
AC_DEFUN([MACRO1], [:macro11:])
AC_DEFUN([MACRO2], [:macro21:])
EOF

cat >m4_2/somedefs.m4 <<EOF
AC_DEFUN([MACRO1], [:macro12:])
EOF

cat >acdir2/macro.m4 <<EOF
AC_DEFUN([MACRO3], [:macro33:])
EOF

$ACLOCAL -I m4_1 -I m4_2
$AUTOCONF
$FGREP ':macro11:' configure
$FGREP ':macro21:' configure
$FGREP ':macro33:' configure
grep MACRO3 aclocal.m4
test ! -e m4_1/macro.m4
test ! -e m4_2/macro.m4

$sleep

$ACLOCAL -I m4_2 -I m4_1
$AUTOCONF
$FGREP ':macro12:' configure
$FGREP ':macro21:' configure
$FGREP ':macro33:' configure
grep MACRO3 aclocal.m4
test ! -e m4_1/macro.m4
test ! -e m4_2/macro.m4

$sleep

$ACLOCAL -I m4_1 -I m4_2 --install
$AUTOCONF
$FGREP ':macro11:' configure
$FGREP ':macro21:' configure
$FGREP ':macro33:' configure
grep MACRO3 aclocal.m4 && exit 1
test -f m4_1/macro.m4
test ! -e m4_2/macro.m4
cp aclocal.m4 copy.m4

$sleep

echo '#GREPME' >>acdir2/macro.m4
$ACLOCAL -I m4_1 -I m4_2 --install
$AUTOCONF
$FGREP ':macro11:' configure
$FGREP ':macro21:' configure
$FGREP ':macro33:' configure
grep MACRO3 aclocal.m4 && exit 1
grep GREPME m4_1/macro.m4 && exit 1
test -f m4_1/macro.m4
test ! -e m4_2/macro.m4
diff aclocal.m4 copy.m4

:
