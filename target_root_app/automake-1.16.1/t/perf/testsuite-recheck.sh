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

# Check performance of recheck target in the face of many failed tests.
# FIXME: this test is not currently able to detect whether the measured
# FIXME: performance is too low, and FAIL accordingly; it just offers an
# FIXME: easy way to verify how effective a performance optimization is.

. test-init.sh

count=5000

cat >> configure.ac <<'END'
AC_OUTPUT
END

cat > Makefile.am <<END
count_expected = $count
TEST_EXTENSIONS = .t
## Updated later.
TESTS =
END

# This should ensure that our timing won't be disturbed by the time
# that would be actually required to run any of:
#  - the test cases
#  - the test driver executing them
#  - the recipe to create the final test-suite log.
cat >> Makefile.am << 'END'
AUTOMAKE_OPTIONS = -Wno-override
## These should never be run.
T_LOG_COMPILER = false
T_LOG_DRIVER = false

# The recipe of this also serves as a sanity check.
$(TEST_SUITE_LOG):
## For debugging.
	@echo "RE-RUN:"; for i in $(TEST_LOGS); do echo "  $$i"; done
## All the test cases should have been re-run.
	@count_got=`for i in $(TEST_LOGS); do echo $$i; done | wc -l` \
	  && echo "Count expected: $(count_expected)" \
	  && echo "Count obtained: $$count_got" \
	  && test $$count_got -eq $(count_expected)
## Pre-existing log files of the tests to re-run should have been
## removed by the 'recheck' target
	@for i in $(TEST_LOGS); do \
	  test ! -f $$i.log || { echo "$$i.log exists!"; exit 1; }; \
	done
## Actually create the target file, for extra safety.
	@echo dummy > $@
END

# Updated later.
: > all

# Temporarily disable shell traces, to avoid bloating the log file.
set +x

for i in $(seq_ 1 $count); do
  echo dummy $i > $i.log
  echo :global-test-result: PASS > $i.trs
  echo :test-result: PASS >> $i.trs
  echo :recheck: yes >> $i.trs
  echo TESTS += $i.t >> Makefile.am
  echo $i >> all
done

# Re-enable shell traces.
set -x

# So that we don't need to create a ton of dummy tests.
echo '$(TESTS):' >> Makefile.am

head -n 100 Makefile.am || : # For debugging.
tail -n 100 Makefile.am || : # Likewise.
cat $count.trs               # Likewise, just the last specimen though.

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE recheck

:
