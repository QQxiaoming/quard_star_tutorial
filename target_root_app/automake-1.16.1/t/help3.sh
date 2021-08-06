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

# Make sure --help and --version takes precedence over other options.

. test-init.sh

# We must have full control over the content of the current directory.
mkdir cleandir
cd cleandir

# Honour user overrides for $ACLOCAL and $AUTOMAKE, but without
# adding extra options.
ACLOCAL=$am_original_ACLOCAL
AUTOMAKE=$am_original_AUTOMAKE

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AC_CONFIG_AUX_DIR([.]) dnl prevent automake from looking into '..'
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_FILES([Makefile])
END

cat > Makefile.am <<END
# So that install-sh will be required.
pkgdata_DATA =
END

$ACLOCAL --force --help --output=foo.m4
test ! -e foo.m4
$ACLOCAL --output=foo.m4 --version --force
test ! -e foo.m4
# Sanity check.
$ACLOCAL --output=foo.m4 --force
test -f foo.m4

mv -f foo.m4 aclocal.m4 # autoconf will need aclocal.m4

$AUTOMAKE --add-missing --help --copy
test ! -e install-sh
$AUTOMAKE --copy --version --add-mising
test ! -e install-sh
# Sanity check.
$AUTOMAKE --add-missing --copy
test -f install-sh

:
