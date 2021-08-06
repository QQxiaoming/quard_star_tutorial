#! /bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

# Test to make sure sub-sub-dirs work correctly.

. test-init.sh

mkdir one
mkdir one/two

cat >> configure.ac << 'END'
AC_CONFIG_FILES([one/Makefile one/two/Makefile])
AC_OUTPUT
END

# Files required because we are using '--gnu'.
: > INSTALL
: > NEWS
: > README
: > COPYING
: > AUTHORS
: > ChangeLog

cat > Makefile.am << 'END'
SUBDIRS = one
END

cat > one/Makefile.am << 'END'
SUBDIRS = two
END

cat > one/two/Makefile.am << 'END'
pkgdata_DATA = data.txt
data.txt:
	echo dummy >$@
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --gnu

./configure
$MAKE
test -f one/two/data.txt

:
