#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Test to make sure that aclocal.m4's dependencies are honored in
# sub-directories.  See also related tests 'remake-subdir*.sh'.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_MACRO_DIR([m4])
FOO_TEST([GREPME])
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
SUBDIRS = sub
END

mkdir sub
: > sub/Makefile.am

mkdir m4
echo 'AC_DEFUN([FOO_TEST], [echo $@])' > m4/moredefs.m4

$ACLOCAL
$AUTOCONF
$AUTOMAKE --copy --add-missing
./configure
$MAKE

# Update an aclocal.m4 dependency, then make sure all Makefiles are
# updated, even from a sub-directory.  Check that AU_ALIAS is
# recognized. Change the definition of FOO_TEST to check that its new
# definition is used.
$sleep # Modified configure dependencies must be newer than config.status.
echo 'AU_ALIAS([FOO_TEST], [AC_SUBST])' > m4/moredefs.m4
cd sub
$MAKE
cd ..
grep GREPME Makefile
grep GREPME sub/Makefile

# Make sure configure dependencies are distributed.
$MAKE distdir
test -f $me-1.0/m4/moredefs.m4

# Change the definition of FOO_TEST to check that its new definition is
# used.  Check that AC_DEFUN_ONCE is caught.
$sleep # Modified configure dependencies must be newer than config.status.
echo 'AC_DEFUN_ONCE([FOO_TEST], [AC_SUBST(__$1__)])' > m4/moredefs.m4
$MAKE
grep 'm4/moredefs\.m4' aclocal.m4
grep '__GREPME__' configure

:
