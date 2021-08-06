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

# ACLOCAL_PATH, acdir and '--install' interactions when serial number
# are involved.

am_create_testdir=empty
. test-init.sh

cat > configure.ac << 'END'
AC_INIT
AM_FOO
END

set_serial ()
{
  serial=$1 file=$2
  sed '/^# serial/d' $file > t
  case $serial in
    none) mv -f t $file;;
       *) (echo "# serial $serial" && cat t) > $file; rm -f t;;
  esac
  cat $file # For debugging.
}

win ()
{
  case $1 in
    sdir) ok=sdir ko=pdir;;
    pdir) ok=pdir ko=sdir;;
       *) fatal_ "win(): invalid argument '$1'";;
  esac
  # This is required on fast machine, to avoid caching and timestamp
  # issues with the autotools (already happened in practice).
  rm -rf configure aclocal.m4 autom4te*.cache m4/*
  $ACLOCAL -I m4 --install
  test -f m4/foo.m4 # Sanity check.
  $AUTOCONF
  $FGREP "::$ok:"  m4/foo.m4
  $FGREP "::$ok::" configure
  $FGREP "::$ko::" m4/foo.m4 configure && exit 1
  :
}

mkdir sdir pdir m4

ACLOCAL="$ACLOCAL --system-acdir=$(pwd)/sdir"
ACLOCAL_PATH=./pdir; export ACLOCAL_PATH

cat > sdir/foo.m4 << 'END'
AC_DEFUN([AM_FOO], [::sdir::])
END
cat > pdir/foo.m4 << 'END'
AC_DEFUN([AM_FOO], [::pdir::])
END

set_serial 2 sdir/foo.m4
set_serial 1 pdir/foo.m4
win sdir

set_serial 3.3 sdir/foo.m4
set_serial 5.7 pdir/foo.m4
win pdir

set_serial 0    sdir/foo.m4
set_serial none pdir/foo.m4
win sdir

set_serial none  sdir/foo.m4
set_serial 1.2.3 pdir/foo.m4
win pdir

:
