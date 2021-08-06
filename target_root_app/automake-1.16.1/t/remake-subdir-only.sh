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

# Make sure remaking rules in subdir are correctly generated.
# See also sister "grepping" test 'remake-subdir-grepping.sh'.

. test-init.sh

fingerprint='=/FiNgErPrInT/='

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

mkdir sub
: > sub/Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

# Rebuild rules should work even if there is no top-level
# makefiles, if we are careful enough to trigger them only
# in proper subdirs.
$sleep
echo "# $fingerprint" > sub/Makefile.am
cd sub
$MAKE Makefile
cd ..
# Check that no spurious Makefile has been created in the
# top-level directory.
test ! -e Makefile
# Check that the Makefile in the subdirectory has been properly
# updated.
$FGREP "$fingerprint" sub/Makefile

:
