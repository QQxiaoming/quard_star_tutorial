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
#  - non-success exit status of a test script is reported in the
#    log file
# See also related test 'tap-passthrough.sh'.

. test-init.sh

cat > Makefile.am << 'END'
TEST_LOG_COMPILER = $(SHELL)
TESTS =
END

exit_statuses='1 2 77 99'

for e in $exit_statuses; do
  unindent > exit-$e.test <<END
    #!/bin/sh
    echo 1..1
    echo ok 1
    exit $e
END
  echo TESTS += exit-$e.test >> Makefile.am
done

. tap-setup.sh

st=0
$MAKE check || st=$?
for e in $exit_statuses; do cat exit-$e.log; done
cat test-suite.log
test $st -gt 0 || exit 1

for e in $exit_statuses; do
  for log in exit-$e.log test-suite.log; do
    grep "^ERROR: exit-$e\\.test - exited with status $e$" $log
  done
done

run_make check TEST_LOG_DRIVER_FLAGS='--ignore-exit'
$FGREP ".test - exited with status" *.log && exit 1

:
