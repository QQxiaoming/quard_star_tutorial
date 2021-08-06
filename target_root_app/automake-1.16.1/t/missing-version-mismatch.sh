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

# Test missing with version mismatches.

am_create_testdir=empty
. test-init.sh

get_shell_script missing

do_check ()
{
  progname=$1; shift;
  ./missing "$@" 2>stderr && { cat stderr >&2; exit 1; }
  cat stderr >&2
  $FGREP "WARNING: '$progname' is probably too old." stderr
}

echo 'AC_INIT([x], [1.0]) AC_PREREQ([9999])' >> configure.ac

do_check autoconf               $AUTOCONF
do_check autoheader             $AUTOHEADER
do_check aclocal-$APIVERSION    $am_original_ACLOCAL

cat > configure.ac << 'END'
AC_INIT([x], [0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
END

echo AUTOMAKE_OPTIONS = 9999.9999 > Makefile.am
$ACLOCAL
: > install-sh
# FIXME: this doesn't work due to a bug in automake (not 'missing').
#do_check automake-$APIVERSION  $am_original_AUTOMAKE

:
