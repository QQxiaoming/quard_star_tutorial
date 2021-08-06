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

# TAP support:
#  - an exit status != 0 of a test script causes an hard error
#  - the '--ignore-exit' option causes the TAP test driver to ignore
#    exit statuses of the test scripts.

. test-init.sh

echo TESTS = > Makefile.am
for st in 1 2 77 99; do
  unindent > exit${st}.test <<END
    #!/bin/sh
    echo 1..1
    echo ok 1
    exit $st
END
  echo TESTS += exit${st}.test >> Makefile.am
done

chmod a+x *.test

. tap-setup.sh

run_make -O -e FAIL check
count_test_results total=8 pass=4 fail=0 xpass=0 xfail=0 skip=0 error=4

grep '^ERROR: exit1\.test - exited with status 1$' stdout
grep '^ERROR: exit2\.test - exited with status 2$' stdout
grep '^ERROR: exit77\.test - exited with status 77$' stdout
grep '^ERROR: exit99\.test - exited with status 99$' stdout

echo TEST_LOG_DRIVER_FLAGS = --ignore-exit >> Makefile
run_make -O check
count_test_results total=4 pass=4 fail=0 xpass=0 xfail=0 skip=0 error=0

:
