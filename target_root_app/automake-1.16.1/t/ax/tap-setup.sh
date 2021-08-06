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

# Auxiliary shell script that copies the common data and files used by
# many tests on TAP support into the current directory.  It should be
# sourced by client test scripts, and assumes the auxiliary test
# 'tap-common-setup.test' has been run beforehand (it will error out
# if this is not the case).

# Check that we are running from a proper directory: last thing we want
# is to overwrite some random user files.
test -f ../../bin/automake && test -f ../../runtest && test -d ../../t \
  || fatal_ "running from a wrong directory"

test ! -f Makefile.am || mv Makefile.am Makefile.am~ \
  || fatal_ "failed to save Makefile.am"

test -d ../tap-common-setup.dir && cp -fpR ../tap-common-setup.dir/* . \
  || fatal_ "couldn't get precomputed data files"

fetch_tap_driver

if test -f Makefile.am~; then
  mv -f Makefile.am~ Makefile.am \
    || fatal_ "failed to restore Makefile.am"
  echo 'TEST_LOG_DRIVER = $(srcdir)/tap-driver' >> Makefile.am \
    || fatal_ "failed to update Makefile.am"
  $AUTOMAKE Makefile \
    || fatal_ "failed to remake Makefile.in"
  ./config.status Makefile \
    || fatal_ "failed to remake Makefile"
fi

:
