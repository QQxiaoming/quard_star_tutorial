#! /bin/sh
# Copyright (C) 2005-2018 Free Software Foundation, Inc.
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

# Test for --install with #serial numbers.

am_create_testdir=empty
. test-init.sh

cat > configure.ac << 'END'
AC_INIT
AM_MACRO1
AM_MACRO2
END

mkdir 1 2 3 4 5 6

cat >1/m1.m4 <<EOF
#serial 1.8.1230.9
AC_DEFUN([AM_MACRO1], [:macro11:])
AC_DEFUN([AM_MACRO2], [:macro21:])
EOF

cat >2/m1.m4 <<EOF
#serial 1.8.1231.9
AC_DEFUN([AM_MACRO1], [:macro12:])
EOF

cat >3/m2.m4 <<EOF
#serial 13
AC_DEFUN([AM_MACRO2], [:macro23:])
EOF

cat >3/m1.m4 <<EOF
#serial 1.8.1230.1
AC_DEFUN([AM_MACRO1], [:macro13:])
EOF

cat >4/mumble.m4 <<EOF
#serial 0
AC_DEFUN([AM_MACRO1], [:macro14:])
EOF

cat >5/ill-formed.m4 <<EOF
#serial bla
#serial .2
#serial
AC_DEFUN([AM_MACRO1], [:macro15:])
EOF

cat >6/after-def.m4 <<EOF
AC_DEFUN([AM_MACRO1], [:macro16:])
#serial 1
EOF

clean_stale ()
{
  rm -rf aclocal.m4 configure autom4te*.cache
}

$ACLOCAL -I 1 -I 2 -I 3 -I 4
$AUTOCONF
$FGREP ':macro11:' configure
$FGREP ':macro21:' configure

clean_stale
$ACLOCAL  -I 1 -I 2 -I 3 -I 4 --install
$AUTOCONF
$FGREP ':macro12:' configure
$FGREP ':macro23:' configure

cp -f aclocal.m4 aclocal-m4.sav
clean_stale
mv -f aclocal-m4.sav aclocal.m4
$ACLOCAL -I 4 -I 1 -I 2 -I 3 --install --dry-run
$AUTOCONF
$FGREP ':macro12:' configure
$FGREP ':macro23:' configure

clean_stale
$ACLOCAL -I 4 -I 1 -I 2 -I 3 --install
$AUTOCONF
$FGREP ':macro14:' configure
$FGREP ':macro23:' configure

clean_stale
$ACLOCAL -I 4 -I 1 -I 2 --install 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
grep 'macro.*AM_MACRO2.*not found' stderr

clean_stale
$ACLOCAL -I 4 -I 1 --install
$AUTOCONF
$FGREP ':macro14:' configure
$FGREP ':macro21:' configure

mkdir acdir
ACLOCAL="$ACLOCAL --system-acdir acdir"

cat >acdir/m1.m4 <<EOF
#serial 456
AC_DEFUN([AM_MACRO1], [:macro1d:])
AC_DEFUN([AM_MACRO2], [:macro2d:])
EOF
clean_stale
$ACLOCAL -I 4 -I 1 --diff=diff >stdout 2>stderr || {
  cat stderr >&2
  cat stdout
  exit 1
}
cat stderr >&2
cat stdout
grep '#serial 456' stdout
test ! -e 4/m1.m4
grep 'installing.*4/m1\.m4' stderr

$ACLOCAL -I 5 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
test $(grep -c 'ill-formed serial' stderr) -eq 3

$ACLOCAL -I 6 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
grep 'serial.*before any macro definition' stderr

:
