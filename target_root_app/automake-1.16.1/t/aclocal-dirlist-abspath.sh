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

# Check use of absolute paths in dirlist.

am_create_testdir=empty
. test-init.sh

mkdir acdir-more sub sub/acdir
echo "$(pwd)/acdir-more" > sub/acdir/dirlist

echo 'AC_DEFUN([AM_FOO], [foo-foo--foo])' > acdir-more/foo.m4

cd sub

cat > configure.ac <<EOF
AC_INIT([$me], [1.0])
AM_FOO
EOF

$ACLOCAL --system-acdir acdir
$AUTOCONF

# Only -I directories are subject to file inclusion.
grep m4_include aclocal.m4 && exit 1

grep 'foo-foo--foo' configure

:
