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

# Make sure that PACKAGE and VERSION are AC_DEFINEd when requested.

. test-init.sh

# -------------------------------------------------------------------
# Do not upgrade this file to use the modern AC_INIT/AM_INIT_AUTOMAKE
# forms.  The day these obsolete AC_INIT and AM_INIT_AUTOMAKE forms
# are dropped, just erase the file.
# nodef2.test contains the modern version of this test.
# -------------------------------------------------------------------

# First, check that PACKAGE and VERSION are output by default.

cat > configure.ac << 'END'
AC_INIT
AM_INIT_AUTOMAKE([UnIqUe_PaCkAgE], [UnIqUe_VeRsIoN])
AC_OUTPUT(output)
END

echo 'DEFS = @DEFS@' > output.in

$ACLOCAL
$AUTOCONF
./configure

grep 'DEFS.*-DVERSION=\\"UnIqUe' output

# Then, check that PACKAGE and VERSION are not output if requested.

cat > configure.ac << 'END'
AC_INIT
AM_INIT_AUTOMAKE([UnIqUe_PaCkAgE], [UnIqUe_VeRsIoN], [no])
AC_OUTPUT(output)
END

$ACLOCAL
$AUTOCONF
./configure

grep 'DEFS.*-DVERSION=\\"UnIqUe' output && exit 1

:
