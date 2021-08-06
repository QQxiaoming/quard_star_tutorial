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
#  - ':test-results:' directives in test scripts' output doesn't
#    originate spurious results in the testsuite summary

. test-init.sh

. tap-setup.sh

cat > all.test <<'END'
1..1
:test-result: PASS
:test-result: FAIL
:test-result: XPASS
:test-result: XFAIL
:test-result: SKIP
:test-result: ERROR
:test-result: UNKNOWN
ok 1
END

run_make -O check
count_test_results total=1 pass=1 fail=0 xpass=0 xfail=0 skip=0 error=0

for result in PASS FAIL XPASS XFAIL SKIP ERROR UNKNOWN; do
  grep "^ *:test-result: $result$" all.log
done

:
