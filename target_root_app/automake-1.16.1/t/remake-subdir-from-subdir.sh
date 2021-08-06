#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Check that remake rules works for adding a new subdirectory from a
# pre-existing subdirectory.
# See also the other similar tests 'remake-subdir*.sh', and the
# related test 'aclocal5.sh'.

. test-init.sh

cat >> configure.ac <<'END'
m4_include([subdirs.m4])
AC_SUBST([MAGIC], [magic])
AC_OUTPUT
END

echo 'AC_CONFIG_FILES([sub/Makefile])' > subdirs.m4
echo 'SUBDIRS = sub' > Makefile.am

mkdir sub
: > sub/Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE

cd sub
$sleep
echo 'AC_CONFIG_FILES([sub/subsub/Makefile])' >> ../subdirs.m4
echo 'SUBDIRS = subsub' >> Makefile.am
mkdir subsub
cat > subsub/Makefile.am <<'END'
all-local:
	: > ok-it-works
END
using_gmake || $MAKE Makefile
$MAKE
test -f subsub/ok-it-works

:
