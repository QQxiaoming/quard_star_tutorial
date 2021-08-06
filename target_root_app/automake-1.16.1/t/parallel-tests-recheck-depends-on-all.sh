#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# The parallel-tests 'recheck' target must depend on the 'all' target.
# See automake bug#11252.

required='cc native'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
all-local:
	test ! -f status || echo okokok > status
bin_PROGRAMS = foo
TESTS = a.test b.test c.test
END

cat > foo.c << 'END'
#include <stdio.h>
int main (void)
{
  printf ("failure :-(" "\n");
  return 1;
}
END

cat > a.test << 'END'
#!/bin/sh
exit 0
END

cat > b.test << 'END'
#!/bin/sh
grep okokok status
END

cat > c.test << 'END'
#!/bin/sh
./foo
END

chmod a+x *.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
./configure

run_make -O -e FAIL check
count_test_results total=3 pass=1 fail=2 skip=0 xfail=0 xpass=0 error=0
test ! -e status

$sleep
: > status
cat > foo.c << 'END'
#include <stdio.h>
int main (void)
{
  printf ("success :-)" "\n");
  return 0;
}
END

run_make -O recheck
count_test_results total=2 pass=2 fail=0 skip=0 xfail=0 xpass=0 error=0
grep '^PASS: b\.test$' stdout
grep '^PASS: c\.test$' stdout

run_make -O recheck
count_test_results total=0 pass=0 fail=0 skip=0 xfail=0 xpass=0 error=0

:
