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

# Make sure aclocal.m4 is rebuilt whenever a configure
# dependency changes.  Test for acinclude.m4 and VPATH too.

. test-init.sh

cat >> configure.ac << 'END'
SOME_DEFS
AC_CONFIG_FILES([sub/Makefile])
AC_CONFIG_MACRO_DIR([m4])
AC_OUTPUT
END

cat > Makefile.am << 'END'
SUBDIRS = sub
END

mkdir sub
: > sub/Makefile.am

mkdir m4
echo 'AC_DEFUN([SOME_DEFS], [])' > m4/somedefs.m4
echo 'AC_DEFUN([MORE_DEFS], [AC_SUBST([GREPME])])' > m4/moredefs.m4

$ACLOCAL
$AUTOCONF
$AUTOMAKE --copy --add-missing

mkdir build
cd build

../configure
$MAKE

# Modified configure dependencies must be newer than config.status.
$sleep
# Update an aclocal.m4 dependency, then make sure all Makefiles
# are updated, even from a sub-directory.
echo 'AC_DEFUN([SOME_DEFS], [MORE_DEFS])' > ../m4/somedefs.m4
# Because aclocal will run again, it should also pick up acinclude.m4.
echo 'AC_SUBST([METOO])' > ../acinclude.m4

cd sub
$MAKE
cd ..
grep GREPME Makefile
grep GREPME sub/Makefile
grep GREPME sub/Makefile
grep METOO Makefile
grep METOO sub/Makefile
grep METOO sub/Makefile

# Make sure configure dependencies are distributed.
$MAKE distdir
test -f $me-1.0/m4/moredefs.m4
test -f $me-1.0/m4/somedefs.m4
test -f $me-1.0/acinclude.m4

:
