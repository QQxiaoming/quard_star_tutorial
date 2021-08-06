#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Ensure we diagnose underquoted AC_DEFUN's.

am_create_testdir=empty
. test-init.sh

cat > configure.ac << 'END'
AC_INIT
FOO
END

mkdir m4
cat >m4/foo.m4 <<EOF
AC_DEFUN(FOO, [echo foo])
EOF

$ACLOCAL -I m4 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
grep 'underquoted definition.*FOO' stderr
grep 'warning.*warning' stderr && exit 1

:
