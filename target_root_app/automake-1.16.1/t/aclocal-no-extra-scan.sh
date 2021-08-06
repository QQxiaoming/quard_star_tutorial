#! /bin/sh
# Copyright (C) 2004-2018 Free Software Foundation, Inc.
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

# Make sure aclocal does not include definitions that are not actually
# evaluated.

am_create_testdir=empty
. test-init.sh

cat > configure.ac << 'END'
AC_INIT([x], [0])
AC_SUBST([POM])
END

mkdir m4
cat >m4/some.m4 <<'EOF'
AC_DEFUN([AM_SOME_MACRO],
[AC_DEFUN([AC_SUBST], [GREPME])])
EOF

$ACLOCAL -I m4
$EGREP 'some.m4|GREPME' aclocal.m4 && exit 1

:
