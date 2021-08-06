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
#  - if a test script exits with non-zero status before printing the TAP
#    plan, then the driver reports both "missing plan" and "exited with
#    non-zero status" errors.

. test-init.sh

cat > foo.test <<END
#!/bin/sh
exit 123
END

echo TESTS = foo.test > Makefile.am

chmod a+x foo.test

. tap-setup.sh

run_make -O -e FAIL check
count_test_results total=2 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=2

grep '^ERROR: foo\.test - exited with status 123$' stdout
grep '^ERROR: foo\.test - missing test plan$' stdout

:
