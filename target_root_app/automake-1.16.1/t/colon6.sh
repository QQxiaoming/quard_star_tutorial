#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Yet another multi-":" test, this time from Ken Pizzini.

. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
# With this, version.good should depend on version.gin, while
# Makefile should not depend on either of them.
AC_CONFIG_FILES([demo/Makefile demo/version.good:demo/version.gin])
AC_OUTPUT
END

mkdir demo
: > demo/version.gin
: > demo/Makefile.am

$ACLOCAL
$AUTOMAKE

# These are older "grepping checks", kept mostly for backward-compatibility.
# They might (unlikely, but possibly) require updating when automake
# internals are changed.  Just relax or remove if they become too fragile.
$EGREP 'Makefile:.*(demo|version)' demo/Makefile.in && exit 1
grep 'version\.good:.*version\.gin' demo/Makefile.in

$AUTOCONF

for vpath in : false; do

  if $vpath; then
    srcdir=..
    mkdir build
    cd build
  else
    srcdir=.
  fi

  $srcdir/configure

  test -f demo/version.good   # Sanity check.
  test ! -s demo/version.good # Likewise.

  cd demo

  $sleep
  # HP-UX make considers targets with exact time stamps as one of their
  # prerequisites out of date.  Ensure Makefile is newer than config.status to
  # avoid triggering the am--refresh rule in the (here-nonexistent) toplevel
  # Makefile.
  touch Makefile

  # version.good should depend on version.gin.
  echo "Rebuilt (srcdir=$srcdir)" > ../$srcdir/demo/version.gin
  $MAKE version.good
  $FGREP "Rebuilt (srcdir=$srcdir)" version.good

  # Makefile should not depend on version.good.
  rm -f version.good
  $MAKE Makefile
  test ! -e version.good

  # version.good should be rebuilt from version.gin.
  $MAKE version.good
  test -f version.good

  # Makefile should not depend on version.gin.
  rm -f ../$srcdir/demo/version.gin
  $MAKE Makefile
  test ! -e ../$srcdir/demo/version.gin # Sanity check.

  # version.good should depend on version.gin.
  rm -f version.good
  run_make -M -e FAIL version.good
  # Try to verify that we errored out for the right reason.
  $FGREP version.gin output

  cd .. # Back in top builddir.
  cd $srcdir

  # Re-create it for the next pass (if any).
  : > demo/version.gin

done

:
