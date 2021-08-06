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

# Check that auxiliary script 'test-driver' doesn't get needlessly
# installed or referenced when the 'serial-tests' option is used.

am_serial_tests=yes
. test-init.sh

cat > Makefile.am <<'END'
AUTOMAKE_OPTIONS = serial-tests
TESTS = foo.test
END

$ACLOCAL

for opts in '' '-a' '--add-missing --copy'; do
  $AUTOMAKE $opts
  $FGREP 'test-driver' Makefile.in && exit 1
  find . | $FGREP 'test-driver' && exit 1
  : For shells with busted 'set -e'.
done

:
