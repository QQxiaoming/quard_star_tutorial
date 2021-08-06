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
#  - an exit status != 0 of a test script causes an hard error, even if
#    the last line of output is a "SKIP plan" (e.g., "1..0 # SKIP").

. test-init.sh

echo TESTS = one.test two.test > Makefile.am

. tap-setup.sh

cat > one.test <<'END'
#!/bin/sh
echo '1..0 # SKIP'
exit 1
END

cat > two.test <<'END'
#!/bin/sh
echo '1..0'
exit 22
END

chmod a+x one.test two.test

run_make -O -e FAIL check

# The 'prove' utility reports both the skip and the non-zero exit status,
# so we do the same.
count_test_results total=4 pass=0 fail=0 xpass=0 xfail=0 skip=2 error=2

grep '^ERROR: one\.test - exited with status 1$' stdout
grep '^ERROR: two\.test - exited with status 22$' stdout

:
