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

# Check precedence rules for ACLOCAL_PATH.

am_create_testdir=empty
. test-init.sh

cat > configure.ac << 'END'
AC_INIT([foo], [1.0])
AM_INIT_AUTOMAKE
FOO_MACRO
BAR_MACRO
AC_PROG_LIBTOOL
AM_GNU_GETTEXT
END

mkdir mdir1 mdir2 mdir3 sysdir extradir

cat > mdir1/foo1.m4 << 'END'
AC_DEFUN([FOO_MACRO], [::pass-foo::])
END

cat > mdir2/foo2.m4 << 'END'
AC_DEFUN([FOO_MACRO], [::fail-foo::])
END

cat > mdir1/baz.m4 << 'END'
AC_DEFUN([BAR_MACRO], [::fail-bar::])
END

cat > mdir3/bar.m4 << 'END'
AC_DEFUN([BAR_MACRO], [::pass-bar::])
END

cat > mdir2/quux.m4 << 'END'
AC_DEFUN([AM_INIT_AUTOMAKE], [::fail-init::])
AC_DEFUN([AC_PROG_LIBTOOL],  [::pass-libtool::])
AC_DEFUN([AM_GNU_GETTEXT],   [::pass-gettext::])
END

cat > sysdir/libtool.m4 << 'END'
AC_DEFUN([AC_PROG_LIBTOOL], [::fail-libtool::])
END

cat > extradir/gettext.m4 << 'END'
AC_DEFUN([AM_GNU_GETTEXT], [::fail-gettext::])
END

echo ./extradir > sysdir/dirlist

ACLOCAL_PATH=mdir1:mdir2 $ACLOCAL -I mdir3 --system-acdir sysdir
$AUTOCONF

$FGREP '::' configure # For debugging.

# Directories coming first in ACLOCAL_PATH should take precedence
# over those coming later.
$FGREP '::pass-foo::' configure

# Directories from '-I' options should take precedence over directories
# in ACLOCAL_PATH.
$FGREP '::pass-bar::' configure

# Directories in ACLOCAL_PATH should take precedence over system acdir
# (typically '${prefix}/share/aclocal'), and any directory added through
# the 'dirlist' special file.
$FGREP '::pass-gettext::' configure
$FGREP '::pass-libtool::' configure

# Directories in ACLOCAL_PATH shouldn't take precedence over the internal
# automake acdir (typically '${prefix}/share/aclocal-${APIVERSION}').
$FGREP 'am__api_version' configure

# A final sanity check.
$FGREP '::fail' configure && exit 1

:
