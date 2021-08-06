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

# Check performanceof recipe generating test-suite.log file and testsuite
# summary.  That has suffered of huge inefficiencies in the past.
# FIXME: this test is not currently able to detect whether the measured
# FIXME: performance is too low, and FAIL accordingly; it just offers
# FIXME: an easy way verify how effective a performance optimization is.

. test-init.sh

count=10000

echo AC_OUTPUT >> configure.ac

cat > Makefile.am <<'END'
TEST_EXTENSIONS = .t
## Will be updated later.
TESTS =
## None of these should ever be run.
T_LOG_COMPILER = false
T_LOG_DRIVER = false
END

# Temporarily disable shell traces, to avoid bloating the log file.
set +x

for i in $(seq_ 1 $count); do
  echo false > $i.t
  echo dummy $i > $i.log
  echo :global-test-result: PASS > $i.trs
  echo :test-result: PASS >> $i.trs
  echo :copy-in-global-log: yes >> $i.trs
  echo TESTS += $i.t
done >> Makefile.am

# Re-enable shell traces.
set -x

head -n 100 Makefile.am || : # For debugging.
tail -n 100 Makefile.am || : # Likewise.
cat $count.trs               # Likewise, just the last specimen though.

# So that we don't need to create a ton of dummy tests.
#echo '$(TESTS):' >> Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

$MAKE test-suite.log >stdout || { cat stdout; exit 1; }
cat stdout

# Sanity checks.
grep "^# TOTAL: $count$" stdout
grep "^dummy $count$" test-suite.log
specimen=347 # Could be any number <= $count.
grep "^dummy $specimen$" test-suite.log

:
