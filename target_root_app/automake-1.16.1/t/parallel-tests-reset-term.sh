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

# Check that the testsuite harness correctly handle overrides of the
# TERM variable by either TESTS_ENVIRONMENT and AM_TESTS_ENVIRONMENT.

required='grep-nonprint'
. test-init.sh

TERM=ansi; export TERM

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
AM_COLOR_TESTS = always
AUTOMAKE_OPTIONS = color-tests
TESTS = foobar
END

cat > foobar << 'END'
#!/bin/sh
echo "TERM='$TERM'"
test x"$TERM" = x"dumb"
END
chmod a+x foobar

mkcheck ()
{
  run_make -O -e IGNORE "$@" check
  cat stdout
  cat foobar.log
  cat test-suite.log
  return $am_make_rc
}

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
./configure

mkcheck TESTS_ENVIRONMENT='TERM=dumb'
cat stdout | grep "PASS.*foobar" | grep "$esc\\["

mkcheck AM_TESTS_ENVIRONMENT='TERM=dumb'
cat stdout | grep "PASS.*foobar" | grep "$esc\\["

:
