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

# Look for a bug where FreeBSD make in concurrent mode reported success
# even when the Automake-generated parallel testsuite harness failed.
# See automake bug#9245.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = foo.test bar.test
END

cat > foo.test <<'END'
#!/bin/sh
exit 1
END
chmod a+x foo.test

cp foo.test bar.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

$MAKE -j1 || skip_ "'$MAKE' doesn't support the -j option"

for j in '' -j1 -j2; do
  $MAKE $j check && exit 1
  run_make -e FAIL -- $j TESTS=foo.test check
  $MAKE $j recheck && exit 1
  run_make -e FAIL -- $j TEST_LOGS=foo.log check
  rm -f test-suite.log
  run_make -e FAIL $j test-suite.log
  test -f test-suite.log || exit 1
done

:
