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

# Test SUDBIRS with '/' in them.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([src/subdir/Makefile src/subdir2/Makefile])
AC_OUTPUT
END

echo SUBDIRS = src/subdir >Makefile.am

mkdir src
mkdir src/subdir
mkdir src/subdir2

: >src/subdir/foo
: >src/subdir2/foo

cat >src/subdir/Makefile.am <<'EOF'
EXTRA_DIST = foo
SUBDIRS = ../subdir2
EOF

cat >src/subdir2/Makefile.am <<'EOF'
EXTRA_DIST = foo
EOF

$ACLOCAL
$AUTOCONF
$AUTOMAKE --copy --add-missing
./configure
$MAKE distdir
test -f $distdir/src/subdir/foo
test -f $distdir/src/subdir2/foo
$MAKE clean
$MAKE distclean
test ! -e src/subdir2/Makefile

:
