#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Test if AM_PROG_AR triggers the use of the ar-lib script.
# This test requires Microsoft lib.
# Keep this test in sync with sister test 'ar-lib5b.sh'.

required=lib
. test-init.sh

cat > configure.ac << END
AC_INIT([$me], [1.0])
AC_CONFIG_AUX_DIR([auxdir])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
# We want to test the content of am_cv_ar_interface in the Makefile.
AC_SUBST([am_cv_ar_interface])
AC_OUTPUT
END

cat > Makefile.am << 'END'
lib_LIBRARIES = libwish.a
libwish_a_SOURCES = wish.c

check-local:
	test x'$(am_cv_ar_interface)' = x'lib'
	test -f ar-lib-worked
	test -f libwish.a
MOSTLYCLEANFILES = ar-lib-worked
END

cat > wish.c << 'END'
int wish(void) { return 0; }
END

mkdir auxdir
cat - "$am_scriptdir/ar-lib" > auxdir/ar-lib << 'END'
#! /bin/sh
:> ar-lib-worked
END
chmod +x auxdir/ar-lib

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

# Sanity check: test that it is ok to use 'am_cv_ar_interface' as we do.
$FGREP 'am_cv_ar_interface=' configure

./configure AR=lib RANLIB=:

$MAKE check
$MAKE distcheck DISTCHECK_CONFIGURE_FLAGS='AR=lib RANLIB=:'

:
