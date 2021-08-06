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

# Test aclocal's '--automake-acdir' and '--system-acdir' options.  Also
# check that stuff in the automake acdir takes precedence over stuff in
# the system acdir.

. test-init.sh

mkdir am sys
# FIXME: remove in Automake 2.0
mkdir am/internal
: > am/internal/ac-config-macro-dirs.m4

cat >> configure.ac <<'END'
MY_MACRO
END

cat > am/foo.m4 <<'END'
AC_DEFUN([AM_INIT_AUTOMAKE], [fake--init--automake])
END

cat > sys/foo.m4 <<'END'
AC_DEFUN([MY_MACRO], [my--macro])
END

$ACLOCAL --automake-acdir am
$AUTOCONF --force
$FGREP 'fake--init--automake' configure
$FGREP 'MY_MACRO' configure

rm -rf autom4te*.cache

$ACLOCAL --system-acdir sys
$AUTOCONF --force
$FGREP 'am__api_version' configure
$FGREP 'my--macro' configure

rm -rf autom4te*.cache

$ACLOCAL --automake-acdir am --system-acdir sys
$AUTOCONF --force
$FGREP 'fake--init--automake' configure
$FGREP 'my--macro' configure

rm -rf autom4te*.cache

$ACLOCAL --system-acdir sys --automake-acdir am
$AUTOCONF --force
$FGREP 'fake--init--automake' configure
$FGREP 'my--macro' configure

rm -rf autom4te*.cache

# Stuff in automake acdir takes precedence over stuff in system acdir.
cat > am/bar.m4 <<'END'
AC_DEFUN([MY_MACRO], [am--macro])
END
$ACLOCAL --automake-acdir am --system-acdir sys
$AUTOCONF --force
$FGREP 'fake--init--automake' configure
$FGREP 'am--macro' configure
$FGREP 'my--macro' configure && exit 1 # Just to be sure.

:
