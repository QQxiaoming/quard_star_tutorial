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

# Sanity check for the automake testsuite.
# Check that our testsuite framework can be instructed not to consider
# an early exit due to some command exiting unexpectedly with status 77.

am_create_testdir=no
. test-init.sh

set +e

unset am_explicit_skips stderr_fileno_

# FIXME: this can be simplified when we improve our runtime checks on the
# testsuite shell to ensure it supports "VAR=val shell_func" correctly.
run_dummy_test ()
{
  env $2 $AM_TEST_RUNNER_SHELL -c "
    am_create_testdir=no
   . test-init.sh
    $1
  " dummy.sh
}

run_dummy_test 'exit 77'
test $? -eq 77 || exit 1

run_dummy_test 'sh -c "exit 77"' am_explicit_skips=no
test $? -eq 77 || exit 1

run_dummy_test '$PERL -e "exit 77"; true' am_explicit_skips=yes
test $? -eq 78 || exit 1

run_dummy_test 'sh -c "exit 77"; exit 0' am_explicit_skips=y
test $? -eq 78 || exit 1

run_dummy_test 'skip_ "foo"; :' am_explicit_skips=y
test $? -eq 77 || exit 1

:
