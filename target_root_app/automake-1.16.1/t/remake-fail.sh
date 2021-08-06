#! /bin/sh
# Copyright (C) 2008-2018 Free Software Foundation, Inc.
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

# Make sure remaking rules fail when they should.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

cat > Makefile.am <<'END'
SUBDIRS = sub
END
mkdir sub
: > sub/Makefile.am

$ACLOCAL
$AUTOMAKE
$AUTOCONF
./configure
$MAKE

$sleep

# Fail for broken input files.

echo "if FOO" > sub/Makefile.am
$MAKE && exit 1

: >sub/Makefile.am
$MAKE

mv Makefile.am backup
echo "if FOO" > Makefile.am
$MAKE && exit 1

# Fail for missing input files, with or without missing
# Makefile.in files.

cp backup Makefile.am
mv sub/Makefile.am sub/backup
$MAKE && exit 1

rm -f sub/Makefile.in
$MAKE && exit 1

mv sub/backup sub/Makefile.am
rm -f Makefile.am
$MAKE && exit 1

rm -f Makefile.in
$MAKE && exit 1

:
