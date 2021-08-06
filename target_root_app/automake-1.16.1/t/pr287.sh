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

# Test for PR 287: empty SUBDIRS.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
SUBDIRS =
bin_SCRIPTS = foo
END

cat > foo << 'END'
#! /bin/sh
echo Maude
END
chmod +x foo

mkdir install

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure "--prefix=$(cd install && pwd)"

$MAKE
$MAKE distdir

:
