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

# Make sure changes to m4_included files also cause aclocal.m4 to change.

. test-init.sh

cat > configure.ac << 'END'
AC_INIT([m4sinclude], [1.0])
AM_INIT_AUTOMAKE
m4_include([somefile.m4])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

cat >Makefile.am <<'EOF'
check-local:
	test -f "$(srcdir)/somefile.m4"
	test -f "$(srcdir)/m4/version1.m4"
	test -f "$(srcdir)/m4/otherfile.m4"
EOF

mkdir m4

echo MACRO1 >somefile.m4
echo 'AC_PREREQ([2.58])' >m4/otherfile.m4

cat >m4/version1.m4 <<EOF
AC_DEFUN([MACRO1])
AC_DEFUN([MACRO2])
m4_sinclude(m4/otherfile.m4)
EOF

cat >m4/version2.m4 <<EOF
AC_DEFUN([MACRO1])
EOF

$ACLOCAL -I m4
grep version2 aclocal.m4
grep version1 aclocal.m4 && exit 1

$sleep
echo MACRO2 >somefile.m4

$ACLOCAL -I m4
grep version2 aclocal.m4 && exit 1
grep version1 aclocal.m4

$sleep
# aclocal.m4 should change if we touch otherfile.m4
touch m4/otherfile.m4
$ACLOCAL -I m4
is_newest aclocal.m4 m4/otherfile.m4

$AUTOCONF
$AUTOMAKE
./configure
$MAKE distcheck

:
