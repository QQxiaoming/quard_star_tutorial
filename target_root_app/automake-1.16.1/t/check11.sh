#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Check skip summary.

# For gen-testsuite-part: ==> try-with-serial-tests <==
. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = skip skip2
END

cat > skip <<'END'
#! /bin/sh
exit 77
END
chmod a+x skip
cp skip skip2

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

run_make -O TESTS=skip check
if test x"$am_serial_tests" = x"yes"; then
  grep '1.*passed' stdout && exit 1
  : For shells with buggy 'set -e'.
else
  count_test_results total=1 pass=0 fail=0 skip=1 xfail=0 xpass=0 error=0
fi

run_make -O TESTS="skip skip2" check
if test x"$am_serial_tests" = x"yes"; then
  grep '2.*passed' stdout && exit 1
  : For shells with buggy 'set -e'.
else
  count_test_results total=2 pass=0 fail=0 skip=2 xfail=0 xpass=0 error=0
fi

:
