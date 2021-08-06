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

# Weak "grepping" test trying to ensure that remaking rules in a subdir
# are correctly generated.
# See also sister functional test 'remake-subdir-only.sh'.

. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

mkdir sub
: > sub/Makefile.am

$ACLOCAL
$AUTOMAKE

grep '^Makefile' sub/Makefile.in
grep '\$(AUTOMAKE).* sub/Makefile' sub/Makefile.in

:
